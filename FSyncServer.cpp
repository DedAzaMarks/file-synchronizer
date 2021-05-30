#include <iostream>

#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Server.h"

int main() {
    Poco::Net::TCPServer srv(new Poco::Net::TCPServerConnectionFactoryImpl<Server>(), 6101);
    srv.start();
    std::cout << "To stop server press \'q\'\n";
    for (;;) {
        int q = getchar();
        if (q == 'q') {
            break;
        }
    }
    srv.stop();
    std::cout << "Server was stoped\n";
    return 0;
}
