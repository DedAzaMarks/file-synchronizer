#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnection.h>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <memory>
#include <filesystem>

#include "types.h"
#include "functions.cpp"

using std::filesystem::path;

#define CHUNK_SIZE 1468 //MTU - 128 bytes

size_t receive_hash_tbl(Client& client, Poco::Net::StreamSocket& ss) {
    uint32_t n;
    uint64_t k = 0;
    size_t v = 0;
    size_t sum = 0;
    sum += ss.receiveBytes(&n, 4);
    while(n--) {
        sum += ss.receiveBytes(&k, 8);
        sum += ss.receiveBytes(&v, 8);
        client.R[k] = v;
    }
    sum += ss.receiveBytes(&n, 4);
    while(n--) {
        sum += ss.receiveBytes(&k, 8);
        sum += ss.receiveBytes(&v, 8);
        client.H[k] = v;
    }
    return sum;
}

size_t send_dd(DiffData& dd, Poco::Net::StreamSocket& ss) {
    size_t sum = 0;
    size_t db_sz = dd.data_blocks.size();
    sum += ss.sendBytes(&db_sz, sizeof(size_t));
    for (size_t i = 0; i < db_sz; ++i) {
         size_t sz = dd.data_blocks[i].data.size();
         sum += ss.sendBytes(&dd.data_blocks[i].offset, sizeof(size_t));
         sum += ss.sendBytes(&sz, sizeof(size_t));
         sum += ss.sendBytes(dd.data_blocks[i].data.data(), sz);
    }
    size_t mb_sz = dd.matched_blocks.size();
    sum += ss.sendBytes(&mb_sz, sizeof(mb_sz));
    for (size_t i = 0; i < mb_sz; ++i) {
        sum += ss.sendBytes(&dd.matched_blocks[i].index, sizeof(size_t));
        sum += ss.sendBytes(&dd.matched_blocks[i].offset, sizeof(size_t));
    }
    return sum;
}

class Server: public Poco::Net::TCPServerConnection {
public:
    Server(const Poco::Net::StreamSocket& s): TCPServerConnection(s) { }
    
    uint8_t begin = 2, end = 251;

    void run() {
        uint32_t b = 0, e = 1;
        Poco::Net::StreamSocket& ss = socket();
        try {
            //size_t sz = ss.receiveBytes(&b, 1);
            uint32_t sz = 0;
            ss.receiveBytes(&b, 4);
            std::cout << "begin\n";
            if (b == end) {
                std::cerr << "Client err\n";
                return;
            }
            ss.receiveBytes(&sz, 4);
            std::unique_ptr<char[]> fileName(new char[sz]);
            ss.receiveBytes(fileName.get(), sz);
            if (!std::filesystem::exists(fileName.get())) {
                ss.sendBytes(&e, 1);
                return;
            } else {
                ss.sendBytes(&b, 1);
            }

            uint32_t chunk_size = 0;
            size_t bufferSize = 0;
            ss.receiveBytes(&chunk_size, 4);
            ss.receiveBytes(&bufferSize, 8);

            std::ifstream file(fileName.get(), std::ios::in | std::ios::binary);
            Client client(chunk_size);
            std::cout << "RECEIVED: " << receive_hash_tbl(client, ss) << '\n';
            DiffData dd;
            std::unique_ptr<char[]> buffer(new char[bufferSize]);
            size_t page = 0;
            while (file) {
                 file.read(buffer.get(), bufferSize);
                 sz = file.gcount();
                 compute_diff(dd, client, buffer, sz, page * bufferSize);
                 page++;
            }
            file.close();

            std::cout << send_dd(dd, ss) << "\n";

            sz = ss.receiveBytes(&e, 1);
            if (e == end) {
                std::cout << "OK\n";
            } else {
                std::cerr << "ERR\n";
            }
        } catch (Poco::Exception& exc) {
            std::cerr << "Smth went wrong\n" << exc.displayText() << '\n';
        }
    }
};

int main(int argc, char* argv[]) {
    Poco::Net::TCPServer srv(new Poco::Net::TCPServerConnectionFactoryImpl<Server>());
    srv.start();
    std::cout << "Server started at port: " << srv.socket().address().port() << '\n';
    std::cout << "To stop server 'q' expected\n";
    for(;;) {
        int q = getchar();
        if (q == 'q')
            break;
    }
    srv.stop();
    std::cout << "server was stoped\n";
    return 0;
}
