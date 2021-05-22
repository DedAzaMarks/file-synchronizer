#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <memory>

//#pragma once
#include <vector>

typedef struct  DataBlock {
    size_t offset;
    std::string data;
} DataBlock;

typedef struct MatchedBlock {
    size_t index;
    size_t offset;
} MatchedBlock;

typedef struct DiffData {
    std::vector<DataBlock> data_blocks;
    std::vector<MatchedBlock> matched_blocks;
} DiffData;

enum { CHUNK_SIZE = 1468 };

void receive_dd(Poco::Net::StreamSocket& ss, DiffData& dd) {
    size_t db_sz = 0;
    ss.receiveBytes(&db_sz, sizeof(db_sz));
    for (size_t i = 0; i < db_sz; ++i) {
        size_t offset = 0;
        ss.receiveBytes(&offset, sizeof(offset));
        size_t sz = 0;
        ss.receiveBytes(&sz, sizeof(sz));
        char buf[sz+1];
        ss.receiveBytes(buf, sz);
        std::string str(buf, sz);
        dd.data_blocks.push_back({offset, str});
    }
    size_t mb_sz = 0;
    ss.receiveBytes(&mb_sz, sizeof(mb_sz));
    for (size_t i = 0; i < mb_sz; ++i) {
        size_t index = 0, offset = 0;
        ss.receiveBytes(&index, sizeof(index));
        ss.receiveBytes(&offset, sizeof(offset));
        dd.matched_blocks.push_back({index, offset});
    }
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "NEED IP AND PORT AND FILE\n";
        exit(1);
    }

    srand(time(NULL));
    Poco::Net::SocketAddress sa(argv[1], strtol(argv[2], NULL, 10));
    Poco::Net::StreamSocket ss(sa);

    uint32_t begin = 2;
    uint8_t end = 251;
    uint32_t res = 0;
    size_t sum = 0;
    std::ifstream bigFile(argv[3], std::ios::in | std::ios::binary);
    if (!bigFile.is_open()) {
        std::cerr << "Failed to open " << argv[3] << "\n";
        exit(1);
    }
    constexpr size_t bufferSize = 1024 * 1024 * 64;
    std::unique_ptr<char[]> buffer(new char[bufferSize]);

    uint32_t sz = strlen(argv[3]);
    ss.sendBytes(&begin, 4);
    ss.sendBytes(&sz, 4);
    ss.sendBytes(argv[3], sz);
    if (res) {
        std::cerr << "Failed\n";
        exit(1);
    }
    while (bigFile) {
        bigFile.read(buffer.get(), bufferSize);
        // process data in buffer
        sz = bigFile.gcount();
        ss.sendBytes(&sz, 4);
        sum += ss.sendBytes(buffer.get(), sz);
        //ss.receiveBytes(&res, 4);
        if (res) {
            size_t len = 0;
            ss.receiveBytes(&len, sizeof(len));
            char buf[len+1];
            ss.receiveBytes(buf, len);
            std::cerr << std::string(buf, len);
            exit(1);
        } else {
            std::cout << "OK\n";
        }
    }
    //DiffData dd;

    //receive_dd(ss, dd);

    // reconstruct data

    ss.sendBytes(&end, 1);
    //std::string data;
    //Пакуем данные(имя файла хэши,...?) здес
    //open file
    std:: cout << "SENT: " << sum << '\n';
    return 0;
}
