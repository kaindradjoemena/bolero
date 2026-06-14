// core/vfs.hpp

#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <string_view>


namespace blr::core
{


class VFS
{
public:
    VFS() = delete;
    ~VFS() = default;

    static void Mount(std::string_view virtPrefix, const std::filesystem::path& physPath);

    static void Unmount(std::string_view virtPrefix);

    static std::filesystem::path Resolve(std::string_view virtPath);


private:
    static inline std::vector<std::pair<std::string, std::filesystem::path>> m_mappings;
};


} /* namespace blr::core */