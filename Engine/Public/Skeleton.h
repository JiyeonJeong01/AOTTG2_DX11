#pragma once

#include "Engine_Define.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagBoneEntry
{
    std::string     strName = "";

    int32_t         iParentBoneIndex = -1;
    std::vector<uint32_t> vecChildBoneIndices;

    _float4x4       matLocalBind{};             /* 모델 파일이 원래 가지고 있던 기준 자세 */
    _float4x4       matCombinedBind{};

    _float4x4       matOffset{}; /* Skinning Offset */
} BONE_ENTRY;

typedef struct ENGINE_DLL tagSkeletonEntry
{
    std::vector<BONE_ENTRY>    bones;

    int32_t                     iRootBoneIndex = -1;
    std::unordered_map<std::string, uint32_t> BoneNameToIndex;
} SKELETON_ENTRY;

NS_END
