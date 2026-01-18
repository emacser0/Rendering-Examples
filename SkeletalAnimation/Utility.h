#pragma once

#include "Core.h"
#include "Assimp.h"

class FUtility
{
public:
    static std::string ReadFile(const std::string& Path);

    static void CheckCompileErrors(GLuint Shader, const std::string& Type);

    static glm::mat4 AssimpToGLM(const aiMatrix4x4& From);
    static glm::vec3 GetGLMVec(const aiVector3D& Vec);
    static glm::quat GetGLMQuat(const aiQuaternion& Orientation);

    static uint32_t TextureFromFile(const char *Path, const std::string& Directory, bool bGamma = false);
};

