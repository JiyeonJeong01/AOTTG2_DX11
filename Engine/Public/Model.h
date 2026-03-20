#pragma once
#include "Component_Struct.h"
#include "Engine_Define.h"
#include "Skeleton.h"
#include "Animation_Clip.h"

typedef struct tagModelPart 
{
    uint32_t        hMesh = INVALID_HANDLE_UINT;       /* 반드시 MeshAsset 핸들(MSB=0) */
    uint32_t        hMaterial = INVALID_HANDLE_UINT;   /* 파트별 재질(또는 슬롯) */
    ASSET_GUID      materialGUID{};

    uint32_t iFirstIndex{}, iIndexCount{};

    /* SKELETON_ENTRY::bones 기준 전역 본 인덱스 */
    std::vector<uint32_t>   vecBoneIndices;             /* 모델의 SKELETON_ENTRY에서 해당 메쉬가 영향을 받는 뼈들의 인덱스 */
    std::vector<_float4x4>  vecOffsetMatrices;          /* vecBoneIndices가 가리키는 Bone의 offsetMatrix이다. pAIBone->mOffsetMatrix */

    _bool Is_Skinned() const noexcept
    {
        return !vecBoneIndices.empty();
    }

}MODEL_PART;

typedef struct tagModelEntry
{
    std::vector<MODEL_PART>             parts;
    ASSET_GUID                          tGUID{};            /* 해당 ModelEntry가 생성된 .model 파일의 GUID */

    SKELETON_ENTRY                      tSkeleton;          /* 모델이 이용하는 전체 Bones */
    std::vector<ANIMATION_CLIP_ENTRY>   vecAnimClips;

    _bool Is_Valid() const noexcept
    {
        return !parts.empty();
    }

    _bool Has_Skeleton() const noexcept
    {
        return tSkeleton.iRootBoneIndex >= 0 && !tSkeleton.bones.empty();
    }

    _bool Has_Animation() const noexcept
    {
        return !vecAnimClips.empty();
    }

}MODEL_ENTRY;
