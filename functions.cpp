#include <fstream>
#include <memory>
#include <fcntl.h>
#include <unistd.h>

#include "Client.cpp"
#include "types.h"

Client file_to_hash(std::ifstream& file, uint32_t chunk_size, size_t bufferSize) {
    std::unique_ptr<char[]> buffer(new char[bufferSize]);
    uint32_t sz = 0;
    Client client(chunk_size);
    uint32_t page = 0;
    while (file) {
        file.read(buffer.get(), bufferSize);
        sz = file.gcount();
        std::string str(buffer.get(), sz);
        client.hash_tbl(buffer, str, 0, page * bufferSize);
        page++;
    }
    return client;
}

void compute_diff(DiffData& dd, Client& client, std::unique_ptr<char[]>& buf, std::string& str, size_t page_offset) {
    std::string substr = "";
    size_t data_offset = 0;
    size_t last = 0;
    size_t i = 0;
    hash_r hr;
    {
      size_t L = std::min<uint32_t>(client.chunk_size, str.size() - i);
      uint64_t r_block = hr.r_block(buf, i, L);
      if (client.R.find(r_block) != client.R.end()) {
          substr = str.substr(i, L);
          uint64_t h = xxh::xxhash<64>(substr);
          if (client.H.find(h) != client.H.end()) {
              size_t index = client.H[h];
              int overlap = 0;
              if (last <= i) {
                  dd.matched_blocks.push_back({index, i + page_offset});
                  last = i + L;
              } else {
                  overlap = 1;
              }
              if (i > data_offset)
                  dd.data_blocks.push_back({data_offset + page_offset,
                          str.substr(data_offset, i - data_offset)});
              if (overlap == 0)
                  data_offset = i + L;
          }
      }
    }
    for (i = 1; i < str.size(); ++i) {
        uint32_t chunk = std::min<uint32_t>(client.chunk_size, str.size() - i);
        uint64_t r = hr.r(buf, i, chunk);
        if (client.R.find(r) != client.R.end()) {
            substr = str.substr(i, chunk);
            uint64_t h = xxh::xxhash<64>(substr);
            //if (client.H[h] != 0) {
            if (client.H.find(h) != client.H.end()) {
                size_t index = client.H[h];
                int overlap = 0;
                if (last <= i) {
                    dd.matched_blocks.push_back({index, i + page_offset});
                    last = i + chunk;
                } else {
                    overlap = 1;
                }
                if (i > data_offset)
                    dd.data_blocks.push_back({data_offset + page_offset,
                            str.substr(data_offset, i - data_offset)});
                if (overlap == 0)
                    data_offset = i + chunk;
            }
        }
    }
    if (i != data_offset)
        dd.data_blocks.push_back({data_offset + page_offset, str.substr(data_offset, i - data_offset)});
}

void reconstruct_data(char* filename, DiffData& data,
                      uint32_t chunk_size, size_t bufferSize) {
    std::string new_str;
    std::ofstream out("NEW FILE", std::ios::out | std::ios::binary | std::ios::trunc);
    size_t i = 0;
    size_t j = 0;
    while (i < data.data_blocks.size() && j < data.matched_blocks.size()) {
        if (data.data_blocks[i].offset < data.matched_blocks[j].offset) {
            out.write(data.data_blocks[i].data.data(), data.data_blocks[i].data.size());
            ++i;
        }
        else {
            int fd = open(filename, O_RDONLY);
            size_t file_sz = lseek(fd, 0L, SEEK_END);
            uint32_t sz;
            if ((file_sz / bufferSize) * bufferSize >  data.matched_blocks[j].index * chunk_size) {
                sz = std::min<uint32_t>(chunk_size,
                                        (file_sz / bufferSize) * bufferSize - data.matched_blocks[j].index * chunk_size);
            } else {
                sz = std::min<uint32_t>(chunk_size, file_sz - data.matched_blocks[j].index * chunk_size);
            }
            std::unique_ptr<char[]> buf(new char[sz]);
            lseek(fd, data.matched_blocks[j].index * chunk_size, SEEK_SET);
            read(fd, buf.get(), sz);
            out.write(buf.get(), sz);
            close(fd);
            //new_str += str.substr(y,
            //                      std::min(chunk_size, str.size() - data.matched_blocks[j].index));
            ++j;
        }
    }

    while (i < data.data_blocks.size()) {
        out.write(data.data_blocks[i].data.data(), data.data_blocks[i].data.size());
        ++i;
    }
    while (j < data.matched_blocks.size()) {
        int fd = open(filename, O_RDONLY);
        size_t file_sz = lseek(fd, 0, SEEK_END);
        uint32_t sz;
        if ((file_sz / bufferSize) * bufferSize >  data.matched_blocks[j].index * chunk_size) {
            sz = std::min<uint32_t>(chunk_size,
                                    (file_sz / bufferSize) * bufferSize - data.matched_blocks[j].index * chunk_size);
        } else {
            sz = std::min<uint32_t>(chunk_size, file_sz - data.matched_blocks[j].index * chunk_size);
        }
        std::unique_ptr<char[]> buf(new char[sz]);
        lseek(fd, data.matched_blocks[j].index * chunk_size, SEEK_SET);

        read(fd, buf.get(), sz);
        out.write(buf.get(), sz);
        close(fd);
        ++j;
    }
    // WRITE FILE
    out.close();
}
