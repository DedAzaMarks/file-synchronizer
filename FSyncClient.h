#include <fstream>
#include <unordered_map>
#include <vector>

#include "Functions.h"
#include "Hasher.h"
#include "Poco/Net/StreamSocket.h"
#include "Types.h"

class FSyncClient {
   private:
    std::string fileName;
    u32 chunkSize, pageSize;
    Hasher hasher = Hasher(0);  // seed
    std::unordered_map<u64, u64> r, h;
    DiffData dd;

    Poco::Net::StreamSocket& ss;

    void clearHash() {
        r.clear();
        h.clear();
    }

   public:
    FSyncClient(std::string&& _fileName, u32 _chunkSize, u32 _pageSize,
                Poco::Net::StreamSocket& _ss)
        : fileName(_fileName), chunkSize(_chunkSize), pageSize(_pageSize), ss(_ss) {}

    void GenerateHash() {
        std::ifstream file(fileName, std::ios::in | std::ios::binary);
        std::vector<char> buf(pageSize);
        u64 page = 0;
        while (file) {
            file.read(buf.data(), pageSize);
            u32 sz = file.gcount();
            if (sz == 0) {
                continue;
            }
            for (u64 i = 0; i < sz; i += chunkSize) {
                u32 L = std::min<u32>(chunkSize, sz - i);
                r[hasher.r_block(buf, i, L)] = i / chunkSize + (pageSize * page) / chunkSize;
                h[hasher.h(buf.data() + i, L)] = i / chunkSize + (pageSize * page) / chunkSize;
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
    }

    void ReconstructFile() {
        std::ofstream out("NEW FILE", std::ios::out | std::ios::binary | std::ios::trunc);
        u64 i = 0;
        u64 j = 0;
        while (i < dd.dataBlocks.size() && j < dd.matchedBlocks.size()) {
            if (dd.dataBlocks[i].offset < dd.matchedBlocks[j].offset) {
                out.write(dd.dataBlocks[i].data.data(), dd.dataBlocks[i].data.size());
                ++i;
            } else {
                std::ifstream in(fileName, std::ios::in | std::ios::binary);
                u64 fileSize = in.tellg();
                u32 sz;
                if ((fileSize / pageSize) * pageSize > dd.matchedBlocks[j].index * chunkSize) {
                    sz = std::min<u32>(chunkSize, (fileSize / pageSize) * pageSize -
                                                      dd.matchedBlocks[j].index * chunkSize);
                } else {
                    sz = std::min<u32>(chunkSize, fileSize - dd.matchedBlocks[j].index * chunkSize);
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
            std::ifstream in(fileName, std::ios::in | std::ios::binary);
            u64 fileSize = in.tellg();
            u32 sz;
            if ((fileSize / pageSize) * pageSize > dd.matchedBlocks[j].index * chunkSize) {
                sz = std::min<u32>(chunkSize, (fileSize / pageSize) * pageSize -
                                                  dd.matchedBlocks[j].index * chunkSize);
            } else {
                sz = std::min<u32>(chunkSize, fileSize - dd.matchedBlocks[j].index * chunkSize);
            }
            std::vector<char> buf(sz);
            in.seekg(dd.matchedBlocks[j].index * chunkSize);
            in.read(buf.data(), sz);
            out.write(buf.data(), sz);
            in.close();
            ++j;
        }
        out.close();
    }
};
