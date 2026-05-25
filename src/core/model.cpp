// core/model.cpp

#include "model.hpp"
#include "shader.hpp"
#include "asset_manager.hpp"

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <utility>


namespace blr::core
{


Model::Model(const std::filesystem::path& filePath, Ref<Shader> defaultShader, AssetManager& assetManager)
    : m_defaultShader(defaultShader)
{
    Assimp::Importer importer;

    const uint32_t flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | 
                           aiProcess_CalcTangentSpace |
                           aiProcess_JoinIdenticalVertices | aiProcess_ValidateDataStructure;

    const aiScene* scene = importer.ReadFile(filePath.string(), flags);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "[Assimp Error] Failed to load model: " << filePath << "\n" 
                  << importer.GetErrorString() << std::endl;
        return;
    }

    m_directory = filePath.parent_path();

    ProcessMaterials(scene, assetManager);
    ProcessNode(scene->mRootNode, scene, assetManager);
}

void Model::ProcessMaterials(const aiScene* scene, AssetManager& assetManager)
{
    m_materials.reserve(scene->mNumMaterials);
    for (size_t i = 0; i < scene->mNumMaterials; i++)
    {
        aiMaterial* aiMat = scene->mMaterials[i];

        auto mat = assetManager.CreateMaterial(m_defaultShader);

        aiString texPathStr;

        // Albedo
        if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPathStr) == AI_SUCCESS ||
            aiMat->GetTexture(aiTextureType_BASE_COLOR, 0, &texPathStr) == AI_SUCCESS)
        {
            std::filesystem::path texPath = m_directory / texPathStr.C_Str();
            mat->albedoMap = assetManager.CreateTex(texPath);
        }

        // Normal
        if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &texPathStr) == AI_SUCCESS)
        {
            std::filesystem::path texPath = m_directory / texPathStr.C_Str();
            mat->normalMap = assetManager.CreateTex(texPath);
        }

        // Metallic / Roughness
        if (aiMat->GetTexture(aiTextureType_UNKNOWN, 0, &texPathStr) == AI_SUCCESS)
        {
            std::filesystem::path texPath = m_directory / texPathStr.C_Str();
            Ref<Tex> packedMap = assetManager.CreateTex(texPath);
            
            mat->metallicMap = packedMap;
            mat->roughnessMap = packedMap;
        }
        else
        {
            if (aiMat->GetTexture(aiTextureType_METALNESS, 0, &texPathStr) == AI_SUCCESS)
                mat->metallicMap = assetManager.CreateTex(m_directory / texPathStr.C_Str());
                
            if (aiMat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texPathStr) == AI_SUCCESS)
                mat->roughnessMap = assetManager.CreateTex(m_directory / texPathStr.C_Str());
        }

        m_materials.emplace_back(mat);
    }
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, AssetManager& assetManager)
{
    // Process meshes attached to the current node
    m_meshes.reserve(node->mNumMeshes);
    for (size_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.emplace_back(ProcessMesh(mesh, assetManager));
    }

    for (size_t i = 0; i < node->mNumChildren; i++)
        ProcessNode(node->mChildren[i], scene, assetManager);
}

Ref<Mesh> Model::ProcessMesh(aiMesh* mesh, AssetManager& assetManager)
{
    // For the Mesh class
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // Fetch vertex data
    vertices.reserve(mesh->mNumVertices);
    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex v;

        // Pos
        v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

        // Normals
        if (mesh->mNormals)
        {
            v.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        }
        else
        {
            v.normal = vec3(0.0f);
        }

        // TexCoords
        if (mesh->mTextureCoords[0])
        {
            v.texCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        
            // Tangent & Bitangent
            if (mesh->HasTangentsAndBitangents())
            {
                v.tangent   = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
                v.bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
            }
        }
        else
        {
            v.texCoords = vec2(0.0f);
            v.tangent   = vec3(0.0f);
            v.bitangent = vec3(0.0f);
        }

        vertices.emplace_back(v);
    }

    // Fetch indices
    indices.reserve(mesh->mNumFaces * 3);   // NOTE: the * 3 is guaranteed since the aiProcess_Triangulate flag is set
    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];

        for (size_t j = 0; j < face.mNumIndices; j++)
            indices.emplace_back(face.mIndices[j]);
    }

    // Assign material
    Ref<Material> mat = nullptr;
    if (mesh->mMaterialIndex < m_materials.size())
        mat = m_materials[mesh->mMaterialIndex];


    return assetManager.CreateMesh(std::move(vertices), std::move(indices), mat);
}


} /* namespace blr::core */