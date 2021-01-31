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

class Client {
private:
    std::string str;
    size_t chunk_size;
public:
    Client(std::string _str, size_t _chunk_size) {
        chunk_size = _chunk_size;
        str = _str;
    }
    
    std::unordered_map<uint64_t, std::string> send_hash_tbl (size_t from, algorithm alg) {
        std::unordered_map<uint64_t, std::string> hash_tbl;
        std::string substr = str.substr(0, from);
        if (from != 0) {
            if (alg == R) {
                hash_tbl[xxh::xxhash<64>(substr)] = substr;
            } else {
                hash_tbl[hash_h(substr)] = substr;
            }
        }
 
        size_t i = 0;
        for (i = from; i < str.size(); i += chunk_size) {
            substr = str.substr(i, chunk_size);

            if (alg == R)
                hash_tbl[xxh::xxhash<64>(substr)] = substr; 
            else if (alg == H)
                hash_tbl[hash_h(substr)] = substr;
        }
        if (i > str.size()) {
            substr = str.substr(i - chunk_size, str.size());
            if (alg == R)
                hash_tbl[xxh::xxhash<64>(substr)] = substr;
            else
                hash_tbl[hash_h(substr)] = substr;
        }
        return hash_tbl;
    }

    void compute_likelihood(std::unordered_map<uint64_t, std::string> tbl_r, std::unordered_map<uint64_t, std::string> tbl_h) {
        std::string substr = "";
        size_t match_counter = 0;
        size_t shift = 0;
        for (size_t i = 0; i < str.size() - chunk_size; ++i) {
            size_t curr_mc = 0;
            size_t curr_s = 0;
            substr = str.substr(i, chunk_size);
            uint64_t r = xxh::xxhash<64>(substr);
            if (tbl_r.find(r) != tbl_r.end()) {
                uint64_t h = hash_h(substr);
                if (tbl_h.find(h) !=tbl_h.end()) {
                    std::cout << "MATCH!!!\n";
                    std::cout << "from: " << i << " to: " << i + chunk_size << "\n\n";
                }
            }
        }
    }
};

int main() {
    Client A = Client("aaabbccddee", 3);
    Client B = Client("aabbccddee", 3);
    auto tbl_r = B.send_hash_tbl(0, R);
    auto tbl_h = B.send_hash_tbl(0, H);
    A.compute_likelihood(tbl_r, tbl_h);
    return 0;
}
