#include <fstream>
#include <memory>

#include "Client.cpp"
#include "types.h"

Client file_to_hash(std::ifstream& file, uint32_t chunk_size, size_t bufferSize) {
    std::unique_ptr<char[]> buffer(new char[bufferSize]);
    uint32_t sz = 0;
    Client client(chunk_size);
    while (file) {
        file.read(buffer.get(), bufferSize);
        sz = file.gcount();
        std::string str(buffer.get(), sz);
        client.hash_tbl(str, 0);
    }
    return client;
}

void compute_diff(DiffData& dd, Client& client, std::string& str) {
    std::string substr = "";
    size_t data_offset = 0;
    size_t last = 0;
    size_t i = 0;
    for (i = 0; i < str.size(); ++i) {
        int64_t chunk = std::min<uint32_t>(client.chunk_size, str.size() - i);
        substr = str.substr(i, chunk);
        uint64_t r = xxh::xxhash<64>(substr);
        if (client.R.find(r) != client.R.end()) {
            uint64_t h = client.hash_h(substr);
            //if (client.H[h] != 0) {
            if (client.H.find(h) != client.H.end()) {
                size_t index = client.H[h];
                int overlap = 0;
                if (last <= i) {
                    dd.matched_blocks.push_back({index, i});
                    last = i + chunk;
                } else {
                    overlap = 1;
                }
                if (i > data_offset)
                    dd.data_blocks.push_back({data_offset, str.substr(data_offset, i - data_offset)});
                if (overlap == 0)
                    data_offset = i + chunk;
            }
        }
    }
    if (i != data_offset)
        dd.data_blocks.push_back({data_offset, str.substr(data_offset, i - data_offset)});
}
