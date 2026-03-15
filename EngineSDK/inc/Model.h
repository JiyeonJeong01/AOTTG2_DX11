#pragma once
#include "Component_Struct.h"
#include "Engine_Define.h"

typedef struct tagModelPart 
{
    uint32_t        hMesh = INVALID_HANDLE_UINT;       /* 반드시 MeshAsset 핸들(MSB=0) */
    uint32_t        hMaterial = INVALID_HANDLE_UINT;   /* 파트별 재질(또는 슬롯) */
    ASSET_GUID      materialGUID{};

    uint32_t iFirstIndex{}, iIndexCount{};

}MODEL_PART;

typedef struct tagModelEntry
{
    std::vector<MODEL_PART> parts;

    ASSET_GUID tGUID{};

    _bool Is_Valid() const noexcept
    {
        return !parts.empty();
    }
}MODEL_ENTRY;
