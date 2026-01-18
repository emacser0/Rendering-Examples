#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 aBoneIDs;
layout (location = 4) in vec4 aWeights;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 100;
uniform mat4 gBones[MAX_BONES];

void main()
{
    mat4 BoneTransform = mat4(0.0);
    
    // Calculate final bone transformation by summing up weighted bone matrices
    bool hasBone = false;
    for (int i = 0 ; i < 4 ; i++)
    {
        if (aBoneIDs[i] == -1) 
            continue;
        if (aBoneIDs[i] >= MAX_BONES) 
        {
            BoneTransform = mat4(1.0);
            break;
        }
        BoneTransform += gBones[aBoneIDs[i]] * aWeights[i];
        hasBone = true;
    }
    
    if (!hasBone || (aWeights[0] == 0.0 && aWeights[1] == 0.0 && aWeights[2] == 0.0 && aWeights[3] == 0.0)) {
        BoneTransform = mat4(1.0);
    }

    vec4 PosL = BoneTransform * vec4(aPos, 1.0);
    gl_Position = projection * view * model * PosL;
    TexCoords = aTexCoords;
}