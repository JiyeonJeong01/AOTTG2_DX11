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

    /* Skinning 용도 정적 매핑 정보 */
    std::vector<uint32_t>   vecBoneIndices;
    std::vector<_float4x4>  vecOffsetMatrices;          /* pAIBone->mOffsetMatrix */

    _bool Is_Skinned() const noexcept
    {
        return !vecBoneIndices.empty();
    }

}MODEL_PART;

typedef struct tagModelEntry
{
    std::vector<MODEL_PART>         parts;
    ASSET_GUID                      tGUID{};            /* 해당 ModelEntry가 생성된 .model 파일의 GUID */

    SKELETON_ENTRY                  tSkeleton;
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
