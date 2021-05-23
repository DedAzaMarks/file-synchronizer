#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
#include "xxhash.hpp"
#include "types.h"

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

    void hash_tbl (std::string& str, size_t from) {
        std::string substr = str.substr(0, from);
        if (from != 0) {
            R[xxh::xxhash<64>(substr)] = 0;
            H[hash_h(substr)] = 0;
        }

        size_t i = 0;
        for (i = from; i < str.size(); i += chunk_size) {
            substr = str.substr(i, std::min<uint32_t>(chunk_size, str.size() - i));

            R[xxh::xxhash<64>(substr)] = i / chunk_size;
            H[hash_h(substr)] = i / chunk_size;
        }
        if (i > str.size()) {
            substr = str.substr(i - chunk_size, str.size());
            R[xxh::xxhash<64>(substr)] = i / chunk_size - 1;
            H[hash_h(substr)] = i / chunk_size - 1;//chunk_size;
        }
    }


    //void reconstruct_data(DiffData data) {
    //    std::string new_str;
    //    size_t i = 0;
    //    size_t j = 0;
    //    while (i < data.data_blocks.size() && j < data.matched_blocks.size()) {
    //        if (data.data_blocks[i].offset < data.matched_blocks[j].offset) {
    //            new_str += data.data_blocks[i].data;
    //            ++i;
    //        }
    //        else {
    //            new_str += str.substr(data.matched_blocks[j].index * chunk_size,
    //                                  std::min(chunk_size, str.size() - data.matched_blocks[j].index));
    //            ++j;
    //        }
    //    }

    //    while (i < data.data_blocks.size()) {
    //        new_str += data.data_blocks[i].data;
    //        ++i;
    //    }
    //    while (j < data.matched_blocks.size()) {
    //        new_str += str.substr(data.matched_blocks[j].index * chunk_size,
    //                              std::min(chunk_size, str.size() - data.matched_blocks[j].index));
    //        ++j;
    //    }
    //    str = new_str;
    //}

    //std::string to_string() {
    //    return str;
    //}
};
