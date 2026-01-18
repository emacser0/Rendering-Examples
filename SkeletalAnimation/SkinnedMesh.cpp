#include "SkinnedMesh.h"

FSkinnedMesh::FSkinnedMesh(const std::vector<FSkinWeightVertex>& InVertices, const std::vector<uint32_t>& InIndices,
                           const std::vector<FTexture>& InTextures)
{
    Vertices = InVertices;
    Indices = InIndices;
    Textures = InTextures;
    SetupMesh();
}

void FSkinnedMesh::Draw(GLuint Shader)
{
    uint32_t DiffuseNr = 1;
    uint32_t SpecularNr = 1;
    uint32_t NormalNr = 1;
    uint32_t HeightNr = 1;

    for (uint32_t i = 0; i < Textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
        std::string Number;
        std::string Name = Textures[i].Type;
        if (Name == "texture_diffuse")
        {
            Number = std::to_string(DiffuseNr++);
        }
        else if (Name == "texture_specular")
        {
            Number = std::to_string(SpecularNr++);
        }
        else if (Name == "texture_normal")
        {
            Number = std::to_string(NormalNr++);
        }
        else if (Name == "texture_height")
        {
            Number = std::to_string(HeightNr++);
        }

        glUniform1i(glGetUniformLocation(Shader, (Name + Number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, Textures[i].Id);
    }
        
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void FSkinnedMesh::SetupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(FSkinWeightVertex), &Vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(uint32_t), &Indices[0], GL_STATIC_DRAW);

    // Vertex Attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FSkinWeightVertex), (void*)offsetof(FSkinWeightVertex, Position));
        
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FSkinWeightVertex), (void*)offsetof(FSkinWeightVertex, Normal));
        
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(FSkinWeightVertex), (void*)offsetof(FSkinWeightVertex, TexCoords));
        
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(FSkinWeightVertex), (void*)offsetof(FSkinWeightVertex, BoneIDs));
        
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(FSkinWeightVertex), (void*)offsetof(FSkinWeightVertex, Weights));

    glBindVertexArray(0);
}
