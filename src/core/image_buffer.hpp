// core/image_buffer.hpp

#pragma once

#include <vector>
#include <cstdint>
#include <string>


namespace blr::core
{


struct ImageBuffer
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 4;
    std::vector<uint8_t> pixels;

    bool IsEmpty() const { return pixels.empty(); }

    bool SaveToFile(const std::string& filepath, int jpgQuality = 90) const;
};


} /* namespace blr::core */