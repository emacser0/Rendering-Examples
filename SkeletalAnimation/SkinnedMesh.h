#pragma once

#include "Core.h"

struct FSkinWeightVertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    int32_t BoneIDs[4];
    float Weights[4];
};

struct FBone
{
    int32_t Id;
    glm::mat4 Offset;
};

class FSkinnedMesh
{
public:
    FSkinnedMesh(const std::vector<FSkinWeightVertex>& InVertices, const std::vector<uint32_t>& InIndices, const std::vector<FTexture>& InTextures);

    void Draw(GLuint Shader);

private:
    void SetupMesh();
    
public:
    std::vector<FSkinWeightVertex> Vertices;
    std::vector<uint32_t> Indices;
    std::vector<FTexture> Textures;
    
private:
    uint32_t VAO;
    uint32_t VBO;
    uint32_t EBO;
};

