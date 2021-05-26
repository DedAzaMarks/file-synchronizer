#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
#include "xxhash.c"
#include "types.h"
#include "hash.cpp"

enum algorithm { R, H };

class Client {
public:
    uint32_t chunk_size;

    std::unordered_map<uint64_t, size_t> R;
    std::unordered_map<uint64_t, size_t> H;

    Client(uint32_t ch_sz) {
        chunk_size = ch_sz;
    }

    void hash_tbl (std::unique_ptr<char[]>& buf, size_t sz, uint32_t shift) {
        hash_r hr;
        size_t i = 0;
        for (i = 0; i < sz; i += chunk_size) {
            uint32_t chunk = std::min<uint32_t>(chunk_size, sz - i);
            R[hr.r_block(buf, i, chunk)] = i / chunk_size + shift / chunk_size;
            XXH64_hash_t hash_h = XXH64(buf.get() + i, chunk, 0);
            H[hash_h] = i / chunk_size + shift / chunk_size;
        }
        //if (i > str.size()) {
        //    substr = str.substr(i - chunk_size, str.size());
        //    uint32_t chunk = std::min<uint32_t>(chunk_size, str.size() - i);
        //    R[hr.r_block(buf, i, chunk)] = i / chunk_size - 1 + shift / chunk_size;
        //    H[hash_h(substr)] = i / chunk_size - 1 + shift / chunk_size;//chunk_size;
        //}
    }
};
