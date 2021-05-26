#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
#include "xxhash.hpp"
#include "types.h"
#include "hash.cpp"

enum algorithm { R, H };

class Client {
public:
    uint32_t chunk_size;
    std::hash<std::string> hash_h;

    std::unordered_map<uint64_t, size_t> R;
    std::unordered_map<uint64_t, size_t> H;

    Client(uint32_t ch_sz) {
        chunk_size = ch_sz;
    }

    void hash_tbl (std::unique_ptr<char[]>& buf, std::string& str, size_t from, uint32_t shift) {
        std::string substr = str.substr(0, from);
        hash_r hr;
        size_t i = 0;
        for (i = from; i < str.size(); i += chunk_size) {
            substr = str.substr(i, std::min<uint32_t>(chunk_size, str.size() - i));
            uint32_t chunk = std::min<uint32_t>(chunk_size, str.size() - i);
            std::cout << substr << " " << hr.r_block(buf, i, chunk) << "\n";
            R[hr.r_block(buf, i, chunk)] = i / chunk_size + shift / chunk_size;
            H[hash_h(substr)] = i / chunk_size + shift / chunk_size;
        }
        if (i > str.size()) {
            substr = str.substr(i - chunk_size, str.size());
            uint32_t chunk = std::min<uint32_t>(chunk_size, str.size() - i);
            R[hr.r_block(buf, i, chunk)] = i / chunk_size - 1 + shift / chunk_size;
            H[hash_h(substr)] = i / chunk_size - 1 + shift / chunk_size;//chunk_size;
        }
    }
};
