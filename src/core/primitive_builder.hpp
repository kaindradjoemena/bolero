// core/primitive_builder.hpp

#pragma once

#include "mesh.hpp"
#include <vector>


namespace blr::core
{


class PrimitiveBuilder
{
public:
    static void BuildQuad(std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices);
    static void BuildCube(std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices);
    static void BuildSphere(uint32_t xSegments, uint32_t ySegments, std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices);
};


} /* namespace blr::core */