#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "xxhash.hpp"

enum algorithm { R, H };

std::hash<std::string> hash_h;

typedef struct  DataBlock {
    int64_t offset;
    std::string data;
} DataBlock;

typedef struct MatchedBlock {
    int64_t index;
    int64_t offset;
} MatchedBlock;

typedef struct DiffData {
    std::vector<DataBlock> data_blocks;
    std::vector<MatchedBlock> matched_blocks;
} DiffData;

class Client {
private:
    std::string str;
    size_t chunk_size;

    int64_t find(const std::vector<uint64_t> &v, uint64_t key) {
        for (int64_t i = 0; i < v.size(); ++i)
            if (v[i] == key)
                return i;
        return -1;
    }

public:
    Client(std::string _str, size_t _chunk_size) {
        chunk_size = _chunk_size;
        str = std::move(_str);
    }
    
    std::vector<uint64_t> send_hash_tbl (size_t from, algorithm alg) {
        std::vector<uint64_t> v;
        std::string substr = str.substr(0, from);
        if (from != 0) {
            if (alg == R) {
                v.push_back(xxh::xxhash<64>(substr));
            } else {
                v.push_back(hash_h(substr));
            }
        }
 
        size_t i = 0;
        for (i = from; i < str.size(); i += chunk_size) {
            substr = str.substr(i, chunk_size);

            if (alg == R)
                v.push_back(xxh::xxhash<64>(substr));
            else if (alg == H)
                v.push_back(hash_h(substr));
        }
        if (i > str.size()) {
            substr = str.substr(i - chunk_size, str.size());
            if (alg == R)
                v.push_back(xxh::xxhash<64>(substr));
            else
                v.push_back(hash_h(substr));
        }
        return v;
    }

    DiffData compute_diff(const std::vector<uint64_t> &v_r, const std::vector<uint64_t> &v_h) {
        std::string substr = "";
        int64_t data_offset = 0;
        int64_t last = -1;
        std::vector<MatchedBlock> matched;
        std::vector<DataBlock> data;
        int64_t i = 0;
        for (i = 0; i < str.size(); ++i) {
            int64_t chunk = std::min(chunk_size, str.size() - i);
            substr = str.substr(i, chunk);
            uint64_t r = xxh::xxhash<64>(substr);
            if (find(v_r, r) != -1) {
                uint64_t h = hash_h(substr);
                int64_t index = find(v_h, h);
                if (index != -1) {
                    if (last <= i) {
                        matched.push_back({index, i});
                        last = i + chunk;
                    }
                    if (i > data_offset)
                        data.push_back({data_offset, str.substr(data_offset, i - data_offset)});
                    data_offset = i + chunk;
                }
            }
        }
        if (i != data_offset)
            data.push_back({data_offset, str.substr(data_offset, i - data_offset)});
//        std::cout << "MATCHED:\n";
//        for (const auto &x : matched)
//            std::cout << x.offset << " " <<x.index <<'\n';
//        std::cout << "DATA:\n";
//        for (const auto &x : data)
//            std::cout << x.offset << " " << x.data <<'\n';
        return {data, matched};
    }

    void reconstruct_data(DiffData data) {
        std::string new_str;
        size_t i = 0;
        size_t j = 0;
        while (i < data.data_blocks.size() && j < data.matched_blocks.size()) {
            if (data.data_blocks[i].offset < data.matched_blocks[j].offset) {
                new_str += data.data_blocks[i].data;
                ++i;
            }
            else {
                new_str += str.substr(data.matched_blocks[j].index * chunk_size,
                                      std::min(chunk_size, str.size() - data.matched_blocks[j].index));
                ++j;
            }
        }

        while (i < data.data_blocks.size()) {
            new_str += data.data_blocks[i].data;
            ++i;
        }
        while (j < data.matched_blocks.size()) {
            new_str += str.substr(data.matched_blocks[j].index * chunk_size,
                                  std::min(chunk_size, str.size() - data.matched_blocks[j].index));
            ++j;
        }
        str = new_str;
    }

    std::string to_string() {
        return str;
    }
};

int main() {
    std::vector<std::pair<Client, Client>> tests = {
            {{"", 3}, {"", 3}},
            {{"", 3}, {"a", 3}},
            {{"", 3}, {"aaa", 3}},
            {{"", 3}, {"aaaaaaaaaaaaaaaa", 3}},
            {{"a", 3}, {"", 3}},
            {{"aaa", 3}, {"", 3}},
            {{"a", 3}, {"b", 3}},
            {{"aaa", 3}, {"bbb", 3}},
            {{"aaaaaaaa", 3}, {"bbbbbbbbbbbbbbbbbb", 3}},
            {{"aaaaaaaaaaaaaaaaaaaaaaa", 3}, {"bbbbbbbbb", 3}},
    };
//    auto tbl_r = B.send_hash_tbl(0, R);
//    auto tbl_h = B.send_hash_tbl(0, H);
//    auto v = A.compute_diff(tbl_r, tbl_h);
//    B.reconstruct_data(v);
//    std:: cout << (B.to_string() == A.to_string() ? "true" : "false") << '\n';
//    std::cout << A.to_string() << '\n';
//    std::cout << B.to_string();

    for (const auto &test : tests) {
        Client A = test.first;
        Client B = test.second;
        auto tbl_r = B.send_hash_tbl(0, R);
        auto tbl_h = B.send_hash_tbl(0, H);
        auto v = A.compute_diff(tbl_r, tbl_h);
        B.reconstruct_data(v);
//        std:: cout << (B.to_string() == A.to_string() ? "true" : "false") << '\n';

        if (B.to_string() != A.to_string()) {
            std::cout << "WA\n";
            std::cout << A.to_string() << '\n' << B.to_string() << '\n';
        } else {
            std::cout << "OK\n";
        }
//    std::cout << A.to_string() << '\n';
//    std::cout << B.to_string();
    }
    return 0;
}
