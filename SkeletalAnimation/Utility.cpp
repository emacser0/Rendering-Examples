#include "Utility.h"
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::string FUtility::ReadFile(const std::string& Path)
{
    std::ifstream File(Path);
    std::stringstream Buffer;
    if (File)
    {
        Buffer << File.rdbuf();
        return Buffer.str();
    }
    else
    {
        std::cout << "Failed to open file: " << Path << std::endl;
        return "";
    }
}

void FUtility::CheckCompileErrors(GLuint Shader, const std::string& Type)
{
    GLint Success;
    GLchar InfoLog[1024];
    if (Type != "PROGRAM")
    {
        glGetShaderiv(Shader, GL_COMPILE_STATUS, &Success);
        if (!Success)
        {
            glGetShaderInfoLog(Shader, 1024, NULL, InfoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << Type << "\n" << InfoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(Shader, GL_LINK_STATUS, &Success);
        if (!Success)
        {
            glGetProgramInfoLog(Shader, 1024, NULL, InfoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << Type << "\n" << InfoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

glm::mat4 FUtility::AssimpToGLM(const aiMatrix4x4& From)
{
    glm::mat4 To;
    To[0][0] = From.a1; To[1][0] = From.a2; To[2][0] = From.a3; To[3][0] = From.a4;
    To[0][1] = From.b1; To[1][1] = From.b2; To[2][1] = From.b3; To[3][1] = From.b4;
    To[0][2] = From.c1; To[1][2] = From.c2; To[2][2] = From.c3; To[3][2] = From.c4;
    To[0][3] = From.d1; To[1][3] = From.d2; To[2][3] = From.d3; To[3][3] = From.d4;
    return To;
}

glm::vec3 FUtility::GetGLMVec(const aiVector3D& Vec)
{ 
    return glm::vec3(Vec.x, Vec.y, Vec.z); 
}

glm::quat FUtility::GetGLMQuat(const aiQuaternion& Orientation)
{
    return glm::quat(Orientation.w, Orientation.x, Orientation.y, Orientation.z);
}

uint32_t FUtility::TextureFromFile(const char *Path, const std::string& Directory, bool bGamma)
{
    std::string Filename(Path);
    Filename = Directory + '/' + Filename;

    uint32_t TextureID;
    glGenTextures(1, &TextureID);

    int32_t Width, Height, NrComponents;
    unsigned char *Data = stbi_load(Filename.c_str(), &Width, &Height, &NrComponents, 0);
    if (Data)
    {
        GLenum Format;
        if (NrComponents == 1)
        {
            Format = GL_RED;
        }
        else if (NrComponents == 3)
        {
            Format = GL_RGB;
        }
        else if (NrComponents == 4)
        {
            Format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, TextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, Format, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(Data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << Filename << std::endl;
        stbi_image_free(Data);
    }

    return TextureID;
}
