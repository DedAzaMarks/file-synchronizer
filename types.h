
#pragma once
#include <vector>

typedef struct  DataBlock {
    size_t offset;
    std::string data;
} DataBlock;

typedef struct MatchedBlock {
    size_t index;
    size_t offset;
} MatchedBlock;

typedef struct DiffData {
    std::vector<DataBlock> data_blocks;
    std::vector<MatchedBlock> matched_blocks;
} DiffData;
