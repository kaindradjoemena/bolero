// core/primitive_builder.cpp

#include "primitive_builder.hpp"

#include <glm/gtc/constants.hpp>


namespace blr::core
{


void PrimitiveBuilder::BuildQuad(std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices)
{
    outVertices = {{ {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
                   { { 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
                   { { 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} },
                   { {-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} }};
    
    outIndices = { 0, 1, 2, 2, 3, 0 };
}

void PrimitiveBuilder::BuildCube(std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices)
{
    vec3 positions[8] = {{-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
                         {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1}};

    int faceIndices[6][4] = {{1, 5, 6, 2},   // Right
                             {4, 0, 3, 7},   // Left
                             {3, 2, 6, 7},   // Top
                             {4, 5, 1, 0},   // Bottom
                             {5, 4, 7, 6},   // Front
                             {0, 1, 2, 3}};  // Back

    vec3 normals[6] = {{ 1,  0,  0}, {-1,  0,  0}, { 0,  1,  0},
                       { 0, -1,  0}, { 0,  0,  1}, { 0,  0, -1}};

    vec2 uvs[4] = { {0,0}, {1,0}, {1,1}, {0,1} };

    outVertices.clear();
    outIndices.clear();

    outVertices.resize(24);
    outIndices.resize(36);

    uint32_t currentVert = 0;
    uint32_t currentIdx  = 0;
    for (uint32_t i = 0; i < 6; i++)
    {
        uint32_t startIdx = currentVert;
        for (uint32_t j = 0; j < 4; j++)
        {
            outVertices[currentVert].position  = positions[faceIndices[i][j]];
            outVertices[currentVert].normal    = normals[i];
            outVertices[currentVert].texCoords = uvs[j];
            outVertices[currentVert].tangent   = vec3(0.0f);
            outVertices[currentVert].bitangent = vec3(0.0f);

            currentVert++;
        }

        outIndices[currentIdx++] = startIdx + 0;
        outIndices[currentIdx++] = startIdx + 1;
        outIndices[currentIdx++] = startIdx + 2;
        outIndices[currentIdx++] = startIdx + 2;
        outIndices[currentIdx++] = startIdx + 3;
        outIndices[currentIdx++] = startIdx + 0;
    }
}

void PrimitiveBuilder::BuildSphere(uint32_t xSegments, uint32_t ySegments, std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices)
{
    uint32_t vertCount = (ySegments + 1) * (xSegments + 1);
    uint32_t idxCount  = ySegments * xSegments * 6;

    outVertices.resize(vertCount);
    outIndices.resize(idxCount);

    uint32_t currentVert = 0;
    for (uint32_t y = 0; y <= ySegments; y++)
    {
        for (uint32_t x = 0; x <= xSegments; x++)
        {
            float xSegment = (float)x / (float)xSegments;
            float ySegment = (float)y / (float)ySegments;

            float xPos = std::cos(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());
            float yPos = std::cos(ySegment *        glm::pi<float>());
            float zPos = std::sin(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());

            outVertices[currentVert].position  = vec3(xPos, yPos, zPos);
            outVertices[currentVert].normal    = glm::normalize(vec3(xPos, yPos, zPos));
            outVertices[currentVert].texCoords = vec2(xSegment, ySegment);
            outVertices[currentVert].tangent   = vec3(0.0f);
            outVertices[currentVert].bitangent = vec3(0.0f);

            currentVert++;
        }
    }

    uint32_t currentIdx = 0;
    for (uint32_t y = 0; y < ySegments; y++)
    {
        for (uint32_t x = 0; x < xSegments; x++)
        {
            outIndices[currentIdx++] = (y + 1) * (xSegments + 1) + x;
            outIndices[currentIdx++] =  y      * (xSegments + 1) + x;
            outIndices[currentIdx++] =  y      * (xSegments + 1) + x + 1;

            outIndices[currentIdx++] = (y + 1) * (xSegments + 1) + x;
            outIndices[currentIdx++] =  y      * (xSegments + 1) + x + 1;
            outIndices[currentIdx++] = (y + 1) * (xSegments + 1) + x + 1;
        }
    }
}


} /* namespace blr::core */