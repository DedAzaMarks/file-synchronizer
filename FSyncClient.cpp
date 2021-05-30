#include "FSyncClient.h"

#include <filesystem>
#include <iostream>
#include <string>

#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "TimerGuard.h"
#include "Types.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Need IP and FILE (you may pass block size as 3rd argument)\n";
        return 1;
    }
    std::string fileName(argv[2], strlen(argv[2]));
    Poco::Net::SocketAddress sa(argv[1], 6101);
    Poco::Net::StreamSocket ss(sa);
    if (!std::filesystem::exists(fileName)) {
        std::cerr << "Failed to open " << fileName << "\n";
        return 1;
    }
    constexpr u32 pageSize = 64 * 1024 * 1024;
    u32 chunkSize = (argc < 4 ? 2048 : std::atoi(argv[3]));

    u32 init = 0, ok = 0, err = 251;

    u64 fileNameSize = fileName.size();
    send(ss, &init, sizeof(init));
    send(ss, &fileNameSize, sizeof(fileNameSize));
    send(ss, fileName.data(), fileNameSize);

    receive(ss, &init, sizeof(init));
    if (init == err) {
        std::cerr << "Remote computer has no sych file\n";
        return 1;
    }

    send(ss, &chunkSize, sizeof(chunkSize));
    send(ss, &pageSize, sizeof(pageSize));

    FSyncClient client(std::move(fileName), chunkSize, pageSize, ss);
    {
        TimerGuard tg("GenerateHash(): chunkSize=" + std::to_string(chunkSize) +
                      " pageSize=" + std::to_string(pageSize));
        client.GenerateHash();
    }
    {
        TimerGuard tg("SendHash()");
        client.SendHash();
    }
    { client.ReceiveDD(); }
    {
        TimerGuard tg("Reconstruct data");
        client.ReconstructFile();
    }

    send(ss, &ok, sizeof(ok));
    return 0;
}
