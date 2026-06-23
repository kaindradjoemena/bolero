// core/image_buffer.cpp

#include "image_buffer.hpp"

#include <stb_image_write.h>
#include <filesystem>
#include <algorithm>
#include <stdexcept>


namespace blr::core
{

    
bool ImageBuffer::SaveToFile(const std::string& filepath, int jpgQuality) const
{
    if (IsEmpty()) return false;

    stbi_flip_vertically_on_write(1);

    std::string ext = std::filesystem::path(filepath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });

    int success = 0;

    if (ext == ".png")
    {
        success = stbi_write_png(filepath.c_str(), width, height, channels, pixels.data(), width * channels);
    } 
    else if (ext == ".jpg" || ext == ".jpeg")
    {
        success = stbi_write_jpg(filepath.c_str(), width, height, channels, pixels.data(), jpgQuality); 
    } 
    else if (ext == ".bmp")
    {
        success = stbi_write_bmp(filepath.c_str(), width, height, channels, pixels.data());
    } 
    else if (ext == ".tga")
    {
        success = stbi_write_tga(filepath.c_str(), width, height, channels, pixels.data());
    } 
    else
    {
        throw std::runtime_error("Unsupported file extension: " + ext);
    }

    if (success == 0)
        throw std::runtime_error("Failed to write image to " + filepath);

    return true;
}


} /* namespace blr::core */
