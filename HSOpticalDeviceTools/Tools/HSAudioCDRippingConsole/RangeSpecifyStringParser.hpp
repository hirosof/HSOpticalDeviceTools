#pragma once

#include <utility>
#include <vector>
#include <string>

using RangeValueItemType = uint32_t;
using RangeValueTypeU = std::pair<RangeValueItemType, RangeValueItemType>;
using RangeValueTypeUVector = std::vector<RangeValueTypeU>;


RangeValueTypeUVector RangeSpecifyStringParseUnsigned(const std::string str );