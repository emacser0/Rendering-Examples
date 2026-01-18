#pragma once

#include "Assimp.h"
#include "Core.h"
#include "Model.h"

struct aiAnimation;

struct FKeyPosition 
{
    glm::vec3 Position;
    float TimeStamp;
};

struct FKeyRotation
{
    glm::quat Orientation;
    float TimeStamp;
};

struct FKeyScale
{
    glm::vec3 Scale;
    float TimeStamp;
};

class FBoneTrack
{
public:
    FBoneTrack(const std::string& InName, int32_t InID, const aiNodeAnim* Channel);

    void Update(float AnimationTime);

    glm::mat4 GetLocalTransform() const { return LocalTransform; }
    std::string GetBoneName() const { return Name; }
    int32_t GetBoneID() const { return ID; }
    
    int32_t GetPositionIndex(float AnimationTime) const;
    int32_t GetRotationIndex(float AnimationTime) const;
    int32_t GetScaleIndex(float AnimationTime) const;

private:
    float GetScaleFactor(float LastTimeStamp, float NextTimeStamp, float AnimationTime) const;

    glm::mat4 InterpolatePosition(float AnimationTime) const;
    glm::mat4 InterpolateRotation(float AnimationTime) const;
    glm::mat4 InterpolateScale(float AnimationTime) const;
    
private:
    std::vector<FKeyPosition> Positions;
    std::vector<FKeyRotation> Rotations;
    std::vector<FKeyScale> Scales;

    glm::mat4 LocalTransform;
    std::string Name;
    int32_t ID;

};

class FAnimation
{
public:
    FAnimation(const std::string& AnimationPath, FModel* Model);
    virtual ~FAnimation() {}

    FBoneTrack* FindBoneTrack(const std::string& Name);

    inline float GetTicksPerSecond() const { return TicksPerSecond; }
    inline float GetDuration() const { return Duration; }
    inline const FAssimpNodeData& GetRootNode() { return RootNode; }
    inline const std::map<std::string, FBone>& GetBoneMap() { return BoneMap; }

private:
    void ReadMissingBones(const aiAnimation* Animation, FModel& Model);
    void ReadHeirarchyData(FAssimpNodeData& Dest, const aiNode* Src);

private:
    float Duration;
    float TicksPerSecond;
    std::vector<FBoneTrack> BoneTracks;
    FAssimpNodeData RootNode;
    std::map<std::string, FBone> BoneMap;
};

class FAnimator
{
public:
    FAnimator(FAnimation* Animation);

    void UpdateAnimation(float InDeltaTime);
    void PlayAnimation(FAnimation* Animation);
    void CalculateBoneTransform(const FAssimpNodeData* Node, glm::mat4 ParentTransform);

    std::vector<glm::mat4> GetFinalBoneMatrices() const { return FinalBoneMatrices; }

private:
    std::vector<glm::mat4> FinalBoneMatrices;
    FAnimation* CurrentAnimation;
    float CurrentTime;
    float DeltaTime;
};

