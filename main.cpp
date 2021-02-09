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

typedef struct Data {
    bool found;
    int64_t from;
    int64_t chunk;
    std::string str;
} Data;

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

    std::vector<Data> compute_likelihood(const std::vector<uint64_t> tbl_r, const std::vector<uint64_t> tbl_h) {
        std::string substr = "";
        int64_t begin = 0;
        std::vector<Data> data;
        for (int64_t i = 0; i < str.size(); ++i) {
            int64_t chunk = std::min(chunk_size, str.size() - i);
            substr = str.substr(i, chunk);
            uint64_t r = xxh::xxhash<64>(substr);
            if (find(tbl_r, r) != -1) {
                uint64_t h = hash_h(substr);
                int64_t indx = find(tbl_h, h);
//                std::cout << substr << ' ' << indx << ' ' << h << " " << tbl_h[indx] << '\n';
                if (indx != -1) {
//                    if (begin < i)
//                        data.push_back({false, -1, i - begin, str.substr(begin, i-begin)});
                    data.push_back({true, indx, i, ""});
                }
            }
        }
        return data;
    }

    void reconstruct_data(std::vector<Data> data) {
        std::string new_str = "";
        for (const auto& elem : data) {
//            if (elem.found == true)
//                new_str.push_back(str.substr())
        }
    }
};

int main() {
    Client A = Client("aaabbccddee", 3);
    Client B = Client("aabbccddee", 3);
    auto tbl_r = B.send_hash_tbl(0, R);
    auto tbl_h = B.send_hash_tbl(0, H);
    auto v = A.compute_likelihood(tbl_r, tbl_h);
    for (auto &x : v) {
        std::cout << (x.found ? "true" : "false") << " block number: " << x.from << " shift: " << x.chunk << " " << x.str << '\n';
    }
    B.reconstruct_data(v);
    return 0;
}
