#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "Functions.h"
#include "Hasher.h"
#include "Poco/Net/StreamSocket.h"
#include "Types.h"

class FSyncClient {
   private:
    std::string fileName;
    u64 chunkSize, pageSize;
    Hasher hasher = Hasher(0);  // seed
    std::unordered_multimap<u64, u64> r, h;
    DiffData dd;

    Poco::Net::StreamSocket& ss;

    void clearHash() {
        r.clear();
        h.clear();
    }

   public:
    FSyncClient(std::string&& _fileName, u64 _chunkSize, u64 _pageSize,
                Poco::Net::StreamSocket& _ss)
        : fileName(_fileName), chunkSize(_chunkSize), pageSize(_pageSize), ss(_ss) {}

    void GenerateHash() {
        std::ifstream file(fileName, std::ios::in | std::ios::binary);
        std::vector<char> buf(pageSize);
        u64 page = 0;
        while (file) {
            file.read(buf.data(), pageSize);
            u64 sz = file.gcount();
            if (sz == 0) {
                continue;
            }
            for (u64 i = 0; i < sz; i += chunkSize) {
                u64 L = std::min<u64>(chunkSize, sz - i);
                r.insert({hasher.r_block(buf, i, L), i / chunkSize + (pageSize * page) / chunkSize});
                h.insert({hasher.h(buf.data() + i, L), i / chunkSize + (pageSize * page) / chunkSize});
            }
            ++page;
        }
        file.close();
    }

    void SendHash() {
        u64 sz = r.size();
        send(ss, &sz, sizeof(sz));
        for (const auto& [k, v] : r) {
            send(ss, &k, sizeof(k));
            send(ss, &v, sizeof(v));
        }
        sz = h.size();
        send(ss, &sz, sizeof(sz));
        for (const auto& [k, v] : h) {
            send(ss, &k, sizeof(k));
            send(ss, &v, sizeof(v));
        }
        std::cout << "Sent: r=" << r.size() << " h=" << h.size () << "\n";
        clearHash();
    }

    void ReceiveDD() {
        u64 dbSize = 0;
        receive(ss, &dbSize, sizeof(dbSize));
        for (u64 i = 0; i < dbSize; ++i) {
            DataBlock db;
            u64 offset = 0, sz = 0;
            receive(ss, &offset, sizeof(offset));
            receive(ss, &sz, sizeof(sz));
            db.offset = offset;
            db.data.resize(sz);
            receive(ss, db.data.data(), sz);
            dd.dataBlocks.push_back(db);
        }
        u64 mbSize = 0;
        receive(ss, &mbSize, sizeof(mbSize));
        for (u64 i = 0; i < mbSize; ++i) {
            MatchedBlock mb;
            u64 index = 0, offset = 0;
            receive(ss, &index, sizeof(index));
            receive(ss, &offset, sizeof(offset));
            mb.index = index;
            mb.offset = offset;
            dd.matchedBlocks.push_back(mb);
        }
        std::cout << "DB: " << dbSize << "\n";
        std::cout << "MB: " << mbSize << "\n";
    }

    void ReconstructFile() {
        std::string newFileName = "~" + fileName + "~";
        std::ofstream out(newFileName, std::ios::out | std::ios::binary | std::ios::trunc);
        u64 i = 0;
        u64 j = 0;
        while (i < dd.dataBlocks.size() && j < dd.matchedBlocks.size()) {
            if (dd.dataBlocks[i].offset < dd.matchedBlocks[j].offset) {
                out.write(dd.dataBlocks[i].data.data(), dd.dataBlocks[i].data.size());
                ++i;
            } else {
                std::ifstream in(fileName, std::ios::ate | std::ios::binary);
                u64 fileSize = in.tellg();
                in.close();
                in = std::ifstream(fileName, std::ios::in | std::ios::binary);
                u64 sz;
                if ((fileSize / pageSize) * pageSize > dd.matchedBlocks[j].index * chunkSize) {
                    sz = std::min<u64>(chunkSize, (fileSize / pageSize) * pageSize -
                                                      dd.matchedBlocks[j].index * chunkSize);
                } else {
                    sz = std::min<u64>(chunkSize, fileSize - dd.matchedBlocks[j].index * chunkSize);
                }
                std::vector<char> buf(sz);
                in.seekg(dd.matchedBlocks[j].index * chunkSize);
                in.read(buf.data(), sz);
                out.write(buf.data(), sz);
                in.close();
                ++j;
            }
        }

        while (i < dd.dataBlocks.size()) {
            out.write(dd.dataBlocks[i].data.data(), dd.dataBlocks[i].data.size());
            ++i;
        }
        while (j < dd.matchedBlocks.size()) {
            std::ifstream in(fileName, std::ios::ate | std::ios::binary);
            u64 fileSize = in.tellg();
            in.close();
            in = std::ifstream(fileName, std::ios::in | std::ios::binary);
            u64 sz;
            if ((fileSize / pageSize) * pageSize > dd.matchedBlocks[j].index * chunkSize) {
                sz = std::min<u64>(chunkSize, (fileSize / pageSize) * pageSize -
                                                  dd.matchedBlocks[j].index * chunkSize);
            } else {
                sz = std::min<u64>(chunkSize, fileSize - dd.matchedBlocks[j].index * chunkSize);
            }
            std::vector<char> buf(sz);
            in.seekg(dd.matchedBlocks[j].index * chunkSize);
            in.read(buf.data(), sz);
            out.write(buf.data(), sz);
            in.close();
            ++j;
        }
        out.close();
        std::string tmpName = std::to_string(
        hasher.h(fileName.data(), fileName.size()));
        std::rename(fileName.data(), tmpName.data());
        std::rename(newFileName.data(), fileName.data());
        std::rename(tmpName.data(), newFileName.data());
    }
};
