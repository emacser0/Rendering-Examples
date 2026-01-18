#include "Model.h"
#include "Assimp.h"
#include "Utility.h"

FModel::FModel(const std::string& Path)
    : BoneCounter(0)
{
    LoadModel(Path);
}

void FModel::Draw(GLuint Shader)
{
    for (uint32_t Index = 0; Index < Meshes.size(); Index++)
    {
        Meshes[Index].Draw(Shader);
    }
}

void FModel::LoadModel(const std::string& Path)
{
    // Check if file exists
    std::ifstream FileStream(Path.c_str());
    if (!FileStream.good())
    {
        std::cout << "ERROR::FILE_NOT_FOUND: " << Path << std::endl;
        return;
    }

    Assimp::Importer Importer;
    const aiScene* Scene = Importer.ReadFile(Path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: " << Importer.GetErrorString() << std::endl;
        return;
    }

    Directory = Path.substr(0, Path.find_last_of('/'));

    std::cout << "Model loaded successfully: " << Path << std::endl;
    std::cout << "Number of meshes: " << Scene->mNumMeshes << std::endl;

    ProcessNode(Scene->mRootNode, Scene);
}

void FModel::ProcessNode(aiNode* Node, const aiScene* Scene)
{
    for (uint32_t Index = 0; Index < Node->mNumMeshes; Index++)
    {
        aiMesh* Mesh = Scene->mMeshes[Node->mMeshes[Index]];
        Meshes.push_back(ProcessMesh(Mesh, Scene));
    }
    
    for (uint32_t Index = 0; Index < Node->mNumChildren; Index++)
    {
        ProcessNode(Node->mChildren[Index], Scene);
    }
}

FSkinnedMesh FModel::ProcessMesh(aiMesh* Mesh, const aiScene* Scene)
{
    std::vector<FSkinWeightVertex> Vertices;
    std::vector<uint32_t> Indices;
    std::vector<FTexture> Textures;

    for (uint32_t i = 0; i < Mesh->mNumVertices; i++)
    {
        FSkinWeightVertex Vertex;
        // Initialize bone data
        for (int32_t j = 0; j < 4; j++)
        {
            Vertex.BoneIDs[j] = -1;
            Vertex.Weights[j] = 0.0f;
        }

        Vertex.Position = glm::vec3(Mesh->mVertices[i].x, Mesh->mVertices[i].y, Mesh->mVertices[i].z);
        if (Mesh->HasNormals())
        {
            Vertex.Normal = glm::vec3(Mesh->mNormals[i].x, Mesh->mNormals[i].y, Mesh->mNormals[i].z);
        }
        if (Mesh->mTextureCoords[0])
        {
            Vertex.TexCoords = glm::vec2(Mesh->mTextureCoords[0][i].x, Mesh->mTextureCoords[0][i].y);
        }
        else
        {
            Vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        Vertices.push_back(Vertex);
    }

    for (uint32_t i = 0; i < Mesh->mNumFaces; i++)
    {
        aiFace Face = Mesh->mFaces[i];
        for (uint32_t j = 0; j < Face.mNumIndices; j++)
        {
            Indices.push_back(Face.mIndices[j]);
        }
    }

    if (Mesh->mMaterialIndex >= 0)
    {
        aiMaterial *Material = Scene->mMaterials[Mesh->mMaterialIndex];
        std::vector<FTexture> DiffuseMaps = LoadMaterialTextures(Material, aiTextureType_DIFFUSE, "texture_diffuse");
        Textures.insert(Textures.end(), DiffuseMaps.begin(), DiffuseMaps.end());
        std::vector<FTexture> SpecularMaps = LoadMaterialTextures(Material, aiTextureType_SPECULAR, "texture_specular");
        Textures.insert(Textures.end(), SpecularMaps.begin(), SpecularMaps.end());
    }

    ExtractBoneWeightForVertices(Vertices, Mesh, Scene);

    return FSkinnedMesh(Vertices, Indices, Textures);
}

std::vector<FTexture> FModel::LoadMaterialTextures(aiMaterial*Mat, aiTextureType Type, const std::string& TypeName)
{
    std::vector<FTexture> Textures;
    for (int32_t i = 0; i < Mat->GetTextureCount(Type); i++)
    {
        aiString Str;
        Mat->GetTexture(Type, i, &Str);
        bool bSkip = false;
        for (int32_t j = 0; j < Textures.size(); j++)
        {
            if (std::strcmp(Textures[j].Path.data(), Str.C_Str()) == 0)
            {
                Textures.push_back(Textures[j]);
                bSkip = true;
                break;
            }
        }
        if (!bSkip)
        {   // if texture hasn't been loaded already, load it
            FTexture Texture;
            Texture.Id = FUtility::TextureFromFile(Str.C_Str(), Directory);
            Texture.Type = TypeName;
            Texture.Path = Str.C_Str();
            Textures.push_back(Texture);
            Textures.push_back(Texture); // add to loaded textures
        }
    }
    return Textures;
}

void FModel::ExtractBoneWeightForVertices(std::vector<FSkinWeightVertex>& Vertices, aiMesh* Mesh, const aiScene* Scene)
{
    for (int32_t i = 0; i < Mesh->mNumBones; ++i)
    {
        int32_t BoneID = -1;
        std::string BoneName = Mesh->mBones[i]->mName.C_Str();
        if (BoneMap.find(BoneName) == BoneMap.end())
        {
            FBone NewBone;
            NewBone.Id = BoneCounter;
            NewBone.Offset = FUtility::AssimpToGLM(Mesh->mBones[i]->mOffsetMatrix);
            BoneMap[BoneName] = NewBone;
            BoneID = BoneCounter;
            BoneCounter++;
        }
        else
        {
            BoneID = BoneMap[BoneName].Id;
        }

        const aiVertexWeight* Weights = Mesh->mBones[i]->mWeights;
        const int32_t NumWeights = Mesh->mBones[i]->mNumWeights;

        for (int32_t j = 0; j < NumWeights; ++j)
        {
            int32_t VertexId = Weights[j].mVertexId;
            float Weight = Weights[j].mWeight;
            SetVertexBoneData(Vertices[VertexId], BoneID, Weight);
        }
    }
}

void FModel::SetVertexBoneData(FSkinWeightVertex& Vertex, int32_t BoneID, float Weight)
{
    for (int32_t Index = 0; Index < 4; ++Index)
    {
        if (Vertex.BoneIDs[Index] < 0)
        {
            Vertex.Weights[Index] = Weight;
            Vertex.BoneIDs[Index] = BoneID;
            break;
        }
    }
}
