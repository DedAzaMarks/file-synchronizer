#include <fstream>
#include <unordered_map>

#include "Functions.h"
#include "Hasher.h"
#include "Poco/Net/StreamSocket.h"
#include "Types.h"

class FSyncServer {
   private:
    std::string fileName;
    u64 chunkSize, pageSize;
    Hasher hasher = Hasher(0);  // seed
    std::unordered_map<u64, u64> r, h;
    DiffData dd;

    Poco::Net::StreamSocket& ss;

   public:
    FSyncServer(std::string _fileName, u32 _chunkSize, u32 _pageSize, Poco::Net::StreamSocket& _ss)
        : fileName(_fileName), chunkSize(_chunkSize), pageSize(_pageSize), ss(_ss) {}

    void GetHash() {
        u64 size = 0, k = 0, v = 0;
        receive(ss, &size, sizeof(size));
        while (size--) {
            receive(ss, &k, sizeof(k));
            receive(ss, &v, sizeof(v));
            r[k] = v;
        }
        receive(ss, &size, sizeof(size));
        while (size--) {
            receive(ss, &k, sizeof(k));
            receive(ss, &v, sizeof(v));
            h[k] = v;
        }
    }

    void ComputeDD() {
        std::ifstream file(fileName, std::ios::in | std::ios::binary);
        std::vector<char> v(pageSize);
        u64 page = 0;
        while (file) {
            file.read(v.data(), pageSize);
            u64 sz = file.gcount();
            if (sz == 0) {
                continue;
            }
            u64 i = 0, last = 0, pageOffset = page * pageSize, dataOffset = 0;
            u32 L = std::min<u32>(chunkSize, sz - i);
            if (r.find(hasher.r_block(v, i, L)) != r.end()) {
                u64 hashH = hasher.h(v.data() + i, L);
                if (h.find(hashH) != h.end()) {
                    u8 overlap = 0;
                    if (last <= i) {
                        MatchedBlock mb = {h[hashH], i + pageOffset};
                        dd.matchedBlocks.push_back(mb);
                        last = i + L;
                    } else {
                        overlap = 1;
                    }
                    if (i > dataOffset) {
                        std::vector<char>::const_iterator start = v.begin() + pageOffset;
                        std::vector<char>::const_iterator finish = v.begin() + i - dataOffset;
                        DataBlock db = {dataOffset + pageOffset, std::vector<char>(start, finish)};
                        dd.dataBlocks.push_back(db);
                    }
                    if (overlap == 0) {
                        dataOffset = i + L;
                    }
                }
            }
            for (i = 1; i < sz; ++i) {
                L = std::min<u32>(chunkSize, sz - i);
                if (r.find(hasher.r(v, i, L)) != r.end()) {
                    u64 hashH = hasher.h(v.data() + i, L);
                    if (h.find(hashH) != h.end()) {
                        u8 overlap = 0;
                        if (last <= i) {
                            MatchedBlock mb = {h[hashH], i + pageOffset};
                            dd.matchedBlocks.push_back(mb);
                            last = i + L;
                        } else {
                            overlap = 1;
                        }
                        if (i > dataOffset) {
                            std::vector<char>::const_iterator start = v.begin() + pageOffset;
                            std::vector<char>::const_iterator finish = v.begin() + i - dataOffset;
                            DataBlock db = {dataOffset + pageOffset,
                                            std::vector<char>(start, finish)};
                            dd.dataBlocks.push_back(db);
                        }
                        if (overlap == 0) {
                            dataOffset = i + L;
                        }
                    }
                }
            }
            if (i != dataOffset) {
                std::vector<char>::const_iterator start = v.begin() + dataOffset;
                std::vector<char>::const_iterator finish = v.begin() + i - dataOffset;
                DataBlock db = {dataOffset + pageOffset, std::vector<char>(start, finish)};
                dd.dataBlocks.push_back(db);
            }
            ++page;
        }
        file.close();
    }

    void SendDD() {
        u64 dbSize = dd.dataBlocks.size();
        send(ss, &dbSize, sizeof(dbSize));
        for (u64 i = 0; i < dbSize; ++i) {
            u64 sz = dd.dataBlocks[i].data.size();
            send(ss, &dd.dataBlocks[i].offset, sizeof(dd.dataBlocks[i].offset));
            send(ss, &sz, sizeof(sz));
            send(ss, dd.dataBlocks[i].data.data(), sz);
        }
        u64 mbSize = dd.matchedBlocks.size();
        send(ss, &mbSize, sizeof(mbSize));
        for (u64 i = 0; i < mbSize; ++i) {
            send(ss, &dd.matchedBlocks[i].index, sizeof(dd.matchedBlocks[i].index));
            send(ss, &dd.matchedBlocks[i].offset, sizeof(dd.matchedBlocks[i].offset));
        }
    }
};
