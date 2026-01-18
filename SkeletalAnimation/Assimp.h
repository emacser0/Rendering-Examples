#pragma once

#include "Core.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct FAssimpNodeData
{
    glm::mat4 Transformation;
    std::string Name;
    int32_t ChildrenCount;
    std::vector<FAssimpNodeData> Children;
};


