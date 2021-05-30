#pragma once

#include <vector>

#include "Poco/Types.h"

using i8 = Poco::Int8;
using i16 = Poco::Int16;
using i32 = Poco::Int32;
using i64 = Poco::Int64;

using u8 = Poco::UInt8;
using u16 = Poco::UInt16;
using u32 = Poco::UInt32;
using u64 = Poco::UInt64;

typedef struct DataBlock {
    u64 offset;
    std::vector<char> data;

} DataBlock;

typedef struct MatchedBlock {
    u64 index;
    u64 offset;

} MatchedBlock;

typedef struct DiffData {
    std::vector<DataBlock> dataBlocks;
    std::vector<MatchedBlock> matchedBlocks;

} DiffData;
