#include "Poco/Net/StreamSocket.h"
#include "Types.h"

u64 receive(Poco::Net::StreamSocket& ss, void* buffer, u64 sz) {
    u64 tmp = 0;
    char* buf = static_cast<char*>(buffer);
    while (tmp != sz) {
        tmp += ss.receiveBytes(buf + tmp, sz - tmp);
    }
    return tmp;
}

u64 send(Poco::Net::StreamSocket& ss, const void* buffer, u64 sz) {
    u64 tmp = 0;
    const char* buf = reinterpret_cast<const char*>(buffer);
    while (tmp != sz) {
        tmp += ss.sendBytes(buf + tmp, sz - tmp);
    }
    return tmp;
}
