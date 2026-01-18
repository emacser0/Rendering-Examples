#pragma once

#include "Core.h"
#include "SkinnedMesh.h"

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

class FModel
{
public:
    FModel(const std::string& Path);

    void Draw(GLuint Shader);
    
    std::map<std::string, FBone>& GetBoneMap() { return BoneMap; }
    int32_t& GetBoneCount() { return BoneCounter; }

private:
    void LoadModel(const std::string& Path);

    void ProcessNode(aiNode* Node, const aiScene* Scene);
    FSkinnedMesh ProcessMesh(aiMesh* Mesh, const aiScene* Scene);

    std::vector<FTexture> LoadMaterialTextures(aiMaterial* Mat, aiTextureType Type, const std::string& TypeName);
    void ExtractBoneWeightForVertices(std::vector<FSkinWeightVertex>& Vertices, aiMesh* Mesh, const aiScene* Scene);
    void SetVertexBoneData(FSkinWeightVertex& Vertex, int32_t BoneID, float Weight);
    
public:
    std::vector<FSkinnedMesh> Meshes;
    std::map<std::string, FBone> BoneMap;
    int32_t BoneCounter;
    
    std::string Directory;
    std::vector<FTexture> Textures;
};

