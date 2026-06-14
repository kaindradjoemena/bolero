// core/vfs.cpp

#include "vfs.hpp"

#include <algorithm>


namespace blr::core
{


void VFS::Mount(std::string_view virtPrefix, const std::filesystem::path& physPath)
{
    m_mappings.emplace_back(virtPrefix, physPath);
}

void VFS::Unmount(std::string_view virtPrefix)
{
    m_mappings.erase(
        std::remove_if(m_mappings.begin(), m_mappings.end(), 
            [&virtPrefix](const auto& pair) { return pair.first == virtPrefix; }),
        m_mappings.end()
    );
}

std::filesystem::path VFS::Resolve(std::string_view virtPath)
{
    for (const auto& [virtPrefix, physPath] : m_mappings)
    {
        if (virtPath.find(virtPrefix) == 0)
        {
            std::string_view relativePath = virtPath.substr(virtPrefix.length());

            return physPath / relativePath;
        }
    }

    return std::filesystem::path(virtPath);
}


} /* namespace blr::core */