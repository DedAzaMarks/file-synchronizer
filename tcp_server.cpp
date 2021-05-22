#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnection.h>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <memory>

#include <filesystem>

#include "types.h"

using std::filesystem::path;

#define CHUNK_SIZE 1468 //MTU - 128 bytes

class Server: public Poco::Net::TCPServerConnection {
public:
    Server(const Poco::Net::StreamSocket& s): TCPServerConnection(s) { }
    
    uint8_t begin = 2, end = 251;

  // void run() {
  //   std::ofstream outfile("output", std::ios::binary);
  //   Poco::Net::StreamSocket& ss = socket();
  //   try {
  //     uint32_t sz;
  //     //[request_sz(4 bytes)][request_data(request_sz bytes)]
  //     int n = ss.receiveBytes(&sz, 4);
  //     char* buffer = new char[sz];
  //     char* bufptr = buffer;
  //     
  //     while (sz > 0) {
  //       if(sz>CHUNK_SIZE){
  //          n = ss.receiveBytes(bufptr,CHUNK_SIZE);
  //          outfile << std::string(bufptr, n);
  //       }else{
  //          n = ss.receiveBytes(bufptr, sz);
  //          outfile << std::string(bufptr, n);
  //       }
  //       sz-=n;
  //       bufptr+=n;   
  //     }
  //     delete[] buffer;
  //   }
  //   catch (Poco::Exception& exc)
  //   { std::cerr << "EchoConnection: " << exc.displayText() << std::endl; }
  // }
    void run() {
        uint32_t b = 0, e = 0;
        uint32_t ok = 0, er = 1;
        Poco::Net::StreamSocket& ss = socket();
        size_t sum = 0;
        try {
            //size_t sz = ss.receiveBytes(&b, 1);
            uint32_t sz = 0;
            uint32_t fileNameSize = 0;
	    std::cout << "begin: " << ss.receiveBytes(&b, 4) << '\n';
	    ss.receiveBytes(&sz, 4);
	    std::unique_ptr<char[]> fileName(new char[sz+1]);
	    ss.receiveBytes(fileName.get(), sz);
	    *(fileName.get()+sz) = '\0';
            std::ofstream out(fileName.get(), std::ios::out | std::ios::binary | std::ios::trunc);
            if (b != 2) {
		std::cerr << "Bad begin\n";
	    }
            while (ss.receiveBytes(&sz, 4) != 1) {
                std::cout << "sz: " << sz << '\n';
                std::unique_ptr<char[]> buffer(new char[1024 * 1024 * 64]);
                size_t tmp = 0;
                while (tmp != sz) {
                    tmp += ss.receiveBytes(buffer.get() + tmp, sz - tmp);
                }
                sum += sz;
                std::cout << "OK\n";
                //ss.sendBytes(&ok, 4);
                
                out.write(buffer.get(), sz);
            }
            out.close();
            //sz = ss.receiveBytes(&fileNameSize, 4);
            //std::cout << fileNameSize << '\n';
            //char buf[fileNameSize];
            //sz = ss.receiveBytes(buf, fileNameSize);
            //fileName = std::string(buf, fileNameSize);
            //std::cout << fileName << '\n';

            //if (!access(buf, F_OK)) {
            //    ss.sendBytes(&ok, 1);
            //} else {
            //    std::cerr << "no file" << fileName << '\n';

            //ss.sendBytes(&er, 1);
            //exit(1);
            
            //    return;
            //}

            
            //DiffData dd;
            //dd.data_blocks.push_back({0, "abcde"});
            //dd.data_blocks.push_back({5, "edcba"});
            //dd.matched_blocks.push_back({0, 10});
            //dd.matched_blocks.push_back({1, 13});
            //size_t db_sz = dd.data_blocks.size();
            //ss.sendBytes(&db_sz, sizeof(size_t));
            //for (size_t i = 0; i < db_sz; ++i) {
            //     size_t sz = dd.data_blocks[i].data.size();
            //     ss.sendBytes(&dd.data_blocks[i].offset, sizeof(size_t));
            //     ss.sendBytes(&sz, sizeof(size_t));
            //     ss.sendBytes(dd.data_blocks[i].data.data(), sz);
            //}
            //size_t mb_sz = dd.matched_blocks.size();
            //ss.sendBytes(&mb_sz, sizeof(mb_sz));
            //for (size_t i = 0; i < mb_sz; ++i) {
            //    ss.sendBytes(&dd.matched_blocks[i].index, sizeof(size_t));
            //    ss.sendBytes(&dd.matched_blocks[i].offset, sizeof(size_t));
            //}

            

            //sz = ss.receiveBytes(&e, 1);
            //if (sz == 1 && e == 251) {
                std::cout << "OK: " << sum << "\n";
            //}
        } catch (Poco::Exception& exc) {
            std::cerr << "Smth went wrong\n" << exc.displayText() << '\n' << "sum: " << sum << '\n';
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
