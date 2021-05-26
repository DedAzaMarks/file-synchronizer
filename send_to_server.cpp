#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <memory>

#include <unordered_map>

//#pragma once
#include <vector>

#include "types.h"
#include "functions.cpp"
#include "Client.cpp"


size_t send_hash_tbl(Client& client, Poco::Net::StreamSocket& ss) {
    size_t sum = 0;
    uint32_t sz = client.R.size();
    sum += ss.sendBytes(&sz, 4);
    for (const auto& [k, v] : client.R) {
        sum += ss.sendBytes(&k, 8);
        sum += ss.sendBytes(&v, 8);
    }
    sz = client.H.size();
    sum += ss.sendBytes(&sz, 4);
    for (const auto& [k, v] : client.H) {
        sum += ss.sendBytes(&k, 8);
        sum += ss.sendBytes(&v, 8);
    }
    return sum;
}

size_t receive_dd(Poco::Net::StreamSocket& ss, DiffData& dd) {
    size_t sum = 0;
    size_t db_sz = 0;
    sum += ss.receiveBytes(&db_sz, sizeof(db_sz));
    for (size_t i = 0; i < db_sz; ++i) {
        size_t offset = 0;
        sum += ss.receiveBytes(&offset, sizeof(offset));
        size_t sz = 0;
        sum += ss.receiveBytes(&sz, sizeof(sz));
        std::unique_ptr<char[]> buf(new char[sz]);
        size_t tmp = 0;
        while (tmp != sz) {
            tmp += ss.receiveBytes(buf.get() + tmp, sz - tmp);
        }
        sum += sz;
        std::string str(buf.get(), sz);
        dd.data_blocks.push_back({offset, str});
    }
    size_t mb_sz = 0;
    sum += ss.receiveBytes(&mb_sz, sizeof(mb_sz));
    for (size_t i = 0; i < mb_sz; ++i) {
        size_t index = 0, offset = 0;
        sum += ss.receiveBytes(&index, sizeof(index));
        sum += ss.receiveBytes(&offset, sizeof(offset));
        dd.matched_blocks.push_back({index, offset});
    }
    return sum;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "NEED: IP ans PORT and FILE\n OPTIONALY: CHUNK_SIZE\n";
        exit(1);
    }

    uint32_t chunk_size = 0;
    if (argc < 4) {
        chunk_size = 256;
    } else {
        chunk_size = strtoll(argv[3], NULL, 10);
    }

    srand(time(NULL));
    Poco::Net::SocketAddress sa(argv[1], 6101);
    Poco::Net::StreamSocket ss(sa);

    uint32_t begin = 2;
    uint8_t end = 251;
    uint32_t res = 0;
    std::ifstream file(argv[2], std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        ss.sendBytes(&end, 1);
        std::cerr << "Failed to open " << argv[2] << "\n";
        exit(1);
    }
    constexpr size_t bufferSize = 64*1024*1024;
    std::unique_ptr<char[]> buffer(new char[bufferSize]);

    uint32_t sz = strlen(argv[2]);
    ss.sendBytes(&begin, 4);
    ss.sendBytes(&sz, 4);
    ss.sendBytes(argv[2], sz);

    ss.receiveBytes(&res, 1);

    if (res == 1) {
        std::cerr << "Remote machine has no such file: " << argv[2] << '\n';
        exit(1);
    }

    ss.sendBytes(&chunk_size, 4);
    ss.sendBytes(&bufferSize, 8);

    Client client = file_to_hash(file, chunk_size, bufferSize);
    std::cout << "SENT: " << send_hash_tbl(client, ss) << '\n';

    DiffData dd;
    std::cout << "RECEIVED: " << receive_dd(ss, dd) << '\n';


    std::cout << "db_sz = " << dd.data_blocks.size() << "\n";
    std::cout << "mb_sz = " << dd.matched_blocks.size() << "\n";

    //for (const auto& x : dd.matched_blocks) {
    //    std::cout << x.offset << " " << x.index << "\n";
    //}
    for (const auto& x : dd.data_blocks) {
        std::cout << x.offset << " " << x.data << "\n";
    }

    reconstruct_data(argv[2], dd, chunk_size, bufferSize);

    ss.sendBytes(&end, 1);

    return 0;
}
