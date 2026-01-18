#include "Animation.h"
#include "Utility.h"
#include "SkinnedMesh.h"

#include "assimp/anim.h"

FBoneTrack::FBoneTrack(const std::string& InName, int32_t InID, const aiNodeAnim* Channel)
    : Name(InName), ID(InID), LocalTransform(1.0f)
{
    for (int32_t PositionIndex = 0; PositionIndex < Channel->mNumPositionKeys; ++PositionIndex)
    {
        aiVector3D AiPosition = Channel->mPositionKeys[PositionIndex].mValue;
        const float TimeStamp = Channel->mPositionKeys[PositionIndex].mTime;
        FKeyPosition Data;
        Data.Position = FUtility::GetGLMVec(AiPosition);
        Data.TimeStamp = TimeStamp;
        Positions.push_back(Data);
    }

    for (int32_t RotationIndex = 0; RotationIndex < Channel->mNumRotationKeys; ++RotationIndex)
    {
        aiQuaternion AiOrientation = Channel->mRotationKeys[RotationIndex].mValue;
        const float TimeStamp = Channel->mRotationKeys[RotationIndex].mTime;
        FKeyRotation Data;
        Data.Orientation = FUtility::GetGLMQuat(AiOrientation);
        Data.TimeStamp = TimeStamp;
        Rotations.push_back(Data);
    }

    for (int32_t KeyIndex = 0; KeyIndex < Channel->mNumScalingKeys; ++KeyIndex)
    {
        aiVector3D Scale = Channel->mScalingKeys[KeyIndex].mValue;
        const float TimeStamp = Channel->mScalingKeys[KeyIndex].mTime;
        FKeyScale Data;
        Data.Scale = FUtility::GetGLMVec(Scale);
        Data.TimeStamp = TimeStamp;
        Scales.push_back(Data);
    }
}

void FBoneTrack::Update(float AnimationTime)
{
    glm::mat4 Translation = InterpolatePosition(AnimationTime);
    glm::mat4 Rotation = InterpolateRotation(AnimationTime);
    glm::mat4 Scale = InterpolateScale(AnimationTime);
    LocalTransform = Translation * Rotation * Scale;
}

int32_t FBoneTrack::GetPositionIndex(float AnimationTime) const
{
    for (int32_t Index = 0; Index < Positions.size() - 1; ++Index)
    {
        if (AnimationTime < Positions[Index + 1].TimeStamp)
        {
            return Index;
        }
    }
        
    assert(0);
    return 0;
}

int32_t FBoneTrack::GetRotationIndex(float AnimationTime) const
{
    for (int32_t Index = 0; Index < Rotations.size() - 1; ++Index)
    {
        if (AnimationTime < Rotations[Index + 1].TimeStamp)
        {
            return Index;
        }
    }
    assert(0);
    return 0;
}

int32_t FBoneTrack::GetScaleIndex(float AnimationTime) const
{
    for (int32_t Index = 0; Index < Scales.size() - 1; ++Index)
    {
        if (AnimationTime < Scales[Index + 1].TimeStamp)
        {
            return Index;
        }
    }
    assert(0);
    return 0;
}

float FBoneTrack::GetScaleFactor(float LastTimeStamp, float NextTimeStamp, float AnimationTime) const
{
    float MidWayLength = AnimationTime - LastTimeStamp;
    float FramesDiff = NextTimeStamp - LastTimeStamp;
    float ScaleFactor = MidWayLength / FramesDiff;
    return ScaleFactor;
}

glm::mat4 FBoneTrack::InterpolatePosition(float AnimationTime) const
{
    if (Positions.size() == 1)
    {
        return glm::translate(glm::mat4(1.0f), Positions[0].Position);
    }

    int32_t P0Index = GetPositionIndex(AnimationTime);
    int32_t P1Index = P0Index + 1;
    float ScaleFactor = GetScaleFactor(Positions[P0Index].TimeStamp, Positions[P1Index].TimeStamp, AnimationTime);
    glm::vec3 FinalPosition = glm::mix(Positions[P0Index].Position, Positions[P1Index].Position, ScaleFactor);
    return glm::translate(glm::mat4(1.0f), FinalPosition);
}

glm::mat4 FBoneTrack::InterpolateRotation(float AnimationTime) const
{
    if (Rotations.size() == 1)
    {
        glm::quat Rotation = glm::normalize(Rotations[0].Orientation);
        return glm::toMat4(Rotation);
    }

    int32_t P0Index = GetRotationIndex(AnimationTime);
    int32_t P1Index = P0Index + 1;
    float ScaleFactor = GetScaleFactor(Rotations[P0Index].TimeStamp, Rotations[P1Index].TimeStamp, AnimationTime);
    glm::quat FinalRotation = glm::slerp(Rotations[P0Index].Orientation, Rotations[P1Index].Orientation, ScaleFactor);
    FinalRotation = glm::normalize(FinalRotation);
    return glm::toMat4(FinalRotation);
}

glm::mat4 FBoneTrack::InterpolateScale(float AnimationTime) const
{
    if (Scales.size() == 1)
    {
        return glm::scale(glm::mat4(1.0f), Scales[0].Scale);
    }

    int32_t P0Index = GetScaleIndex(AnimationTime);
    int32_t P1Index = P0Index + 1;
    float ScaleFactor = GetScaleFactor(Scales[P0Index].TimeStamp, Scales[P1Index].TimeStamp, AnimationTime);
    glm::vec3 FinalScale = glm::mix(Scales[P0Index].Scale, Scales[P1Index].Scale, ScaleFactor);
    return glm::scale(glm::mat4(1.0f), FinalScale);
}

FAnimation::FAnimation(const std::string& AnimationPath, FModel* Model)
{
    Assimp::Importer Importer;
    const aiScene* Scene = Importer.ReadFile(AnimationPath, aiProcess_Triangulate);
    assert(Scene && Scene->mRootNode);
    const aiAnimation* Animation = Scene->mAnimations[0];
    Duration = static_cast<float>(Animation->mDuration);
    TicksPerSecond = static_cast<float>(Animation->mTicksPerSecond);
    ReadHeirarchyData(RootNode, Scene->mRootNode);
    ReadMissingBones(Animation, *Model);
}

FBoneTrack* FAnimation::FindBoneTrack(const std::string& Name)
{
    auto Iter = std::find_if(BoneTracks.begin(), BoneTracks.end(),
    [&](const FBoneTrack& BoneTrack)
    {
     return BoneTrack.GetBoneName() == Name;
    });
        
    if (Iter == BoneTracks.end())
    {
        return nullptr;
    }
    else
    {
        return &(*Iter);
    }
}

void FAnimation::ReadMissingBones(const aiAnimation* Animation, FModel& Model)
{
    uint32_t Size = Animation->mNumChannels;
        
    std::map<std::string, FBone>& ModelBoneMap = Model.GetBoneMap(); 
    int32_t& BoneCount = Model.GetBoneCount(); 

    for (uint32_t i = 0; i < Size; i++)
    {
        const aiNodeAnim* Channel = Animation->mChannels[i];
        std::string BoneName = Channel->mNodeName.data;

        if (ModelBoneMap.find(BoneName) == ModelBoneMap.end())
        {
            ModelBoneMap[BoneName].Id = BoneCount;
            BoneCount++;
        }
        
        BoneTracks.emplace_back(Channel->mNodeName.data, ModelBoneMap[BoneName].Id, Channel);
    }

    BoneMap = ModelBoneMap;
}

void FAnimation::ReadHeirarchyData(FAssimpNodeData& Dest, const aiNode* Src)
{
    assert(Src);

    Dest.Name = Src->mName.data;
    Dest.Transformation = FUtility::AssimpToGLM(Src->mTransformation);
    Dest.ChildrenCount = Src->mNumChildren;

    for (int32_t i = 0; i < Src->mNumChildren; i++)
    {
        FAssimpNodeData NewData;
        ReadHeirarchyData(NewData, Src->mChildren[i]);
        Dest.Children.push_back(NewData);
    }
}

FAnimator::FAnimator(FAnimation* Animation)
    : CurrentAnimation(Animation)
    , CurrentTime(0.0f)
    , DeltaTime(0.0f)
{

    FinalBoneMatrices.reserve(100);
    for (int32_t i = 0; i < 100; i++)
    {
        FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }
}

void FAnimator::UpdateAnimation(float InDeltaTime)
{
    DeltaTime = InDeltaTime;
    if (CurrentAnimation)
    {
        CurrentTime += CurrentAnimation->GetTicksPerSecond() * InDeltaTime;
        CurrentTime = fmod(CurrentTime, CurrentAnimation->GetDuration());
        CalculateBoneTransform(&CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
    }
}

void FAnimator::PlayAnimation(FAnimation* Animation)
{
    CurrentAnimation = Animation;
    CurrentTime = 0.0f;
}

void FAnimator::CalculateBoneTransform(const FAssimpNodeData* Node, glm::mat4 ParentTransform)
{
    assert(Node != nullptr);
    
    glm::mat4 NodeTransform = Node->Transformation;

    FBoneTrack* BoneTrack = CurrentAnimation->FindBoneTrack(Node->Name);
    if (BoneTrack != nullptr)
    {
        BoneTrack->Update(CurrentTime);
        NodeTransform = BoneTrack->GetLocalTransform();
    }

    const glm::mat4 GlobalTransformation = ParentTransform * NodeTransform;
        
    std::map<std::string, FBone> AnimationBoneMap = CurrentAnimation->GetBoneMap();
    if (AnimationBoneMap.find(Node->Name) != AnimationBoneMap.end())
    {
        int32_t Index = AnimationBoneMap[Node->Name].Id;
        glm::mat4 Offset = AnimationBoneMap[Node->Name].Offset;
        FinalBoneMatrices[Index] = GlobalTransformation * Offset;
    }

    for (int32_t i = 0; i < Node->ChildrenCount; i++)
    {
        CalculateBoneTransform(&Node->Children[i], GlobalTransformation);
    }
}
