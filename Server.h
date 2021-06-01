#include <filesystem>
#include <iostream>

#include "FSyncServer.h"
#include "Poco/Net/TCPServerConnection.h"
#include "TimerGuard.h"

class Server : public Poco::Net::TCPServerConnection {
   public:
    Server(const Poco::Net::StreamSocket& s) : TCPServerConnection(s) {}

    u32 err = 251, ok = 0;

    void run() {
        Poco::Net::StreamSocket& ss = socket();

        std::string fileName;
        u32 init = 0;
        receive(ss, &init, sizeof(init));
        if (init == err) {
            std::cerr << "Client err\n";
            return;
        } else {
            std::cout << "Begin\n";
        }
        u64 fileNameSize = 0;
        receive(ss, &fileNameSize, sizeof(fileNameSize));
        fileName.resize(fileNameSize);
        receive(ss, fileName.data(), fileNameSize);

        if (!std::filesystem::exists(fileName)) {
            send(ss, &err, sizeof(err));
            return;
        } else {
            send(ss, &ok, sizeof(ok));
        }

        u64 chunkSize = 0, pageSize = 0;
        receive(ss, &chunkSize, sizeof(chunkSize));
        receive(ss, &pageSize, sizeof(pageSize));
        {
            FSyncServer server(fileName, chunkSize, pageSize, ss);
            {
                TimerGuard tg("GetHash");
                server.GetHash();
            }
            {
                TimerGuard tg("ComputeDD");
                server.ComputeDD();
            }
            {
                TimerGuard tg("SendDD");
                server.SendDD();
            }
        }
        receive(ss, &init, sizeof(init));
        if (init == ok) {
            std::cout << "OK\n";
        } else {
            std::cout << "ERR\n";
        }
        try {
        } catch (Poco::Exception& exc) {
            std::cerr << "Smth went wrong\n" << exc.displayText() << '\n';
        }
    }
};
