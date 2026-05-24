// utils/uuid.hpp

#pragma once

#include <cstdint>
#include <atomic>

namespace blr::core
{

using AssetHandle = uint64_t;

class UUID
{
public:
    static AssetHandle Generate()
    {
        static std::atomic<uint64_t> s_counter{ 1 };
        return s_counter++;
    }
};

} /* namespace blr::core */