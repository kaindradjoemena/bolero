// core/resource.hpp

#pragma once

#include <cstdint>


namespace blr::core
{


using AssetHandle = uint64_t;

class Resource
{
public:
    virtual ~Resource() = default; 

    AssetHandle GetHandle() const { return m_handle; }

private:
    AssetHandle m_handle{0};

// Handle generation must and could only be called by the AssetManager class 
friend class AssetManager;
protected:
    void SetHandle(AssetHandle handle)
    {
        m_handle = handle;
    }
};

}