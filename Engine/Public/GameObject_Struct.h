#pragma once

#include <cstdint>
#include <vector>

#include "Engine_Enum.h"
#include "Engine_Macro.h"
#include "Engine_Typedef.h"

NS_BEGIN(Engine)

typedef struct tagGameObjectHandle
{
    uint32_t    iIndex = 0;
    uint32_t    iVersion = 0;

    bool IsValid() const
    {
        return iIndex != 0;
    }
    bool operator==(const tagGameObjectHandle& other) const
    {
        return iIndex == other.iIndex && iVersion == other.iVersion;
    }
    bool operator!=(const tagGameObjectHandle& other) const
    {
        return !(*this == other);
    }

}GAMEOBJECT_HANDLE;

typedef struct tagGameObjectData
{
    uint32_t    iVersion = 1;       /* slot's current version */
    _bool       bActive = false;
    _bool       bPendingDestroy = false;

    /* For layer access in O(1) */
    Layer::LAYER_ID layer = Layer::INVALID_LAYER;
    uint32_t iIndexInLayer = 0;

    /* Components */
    uint32_t iComponentSlots[COMPONENT_MAX] = { 0, };
    Component::COMPONENT_MASK   componentMask = 0;

    /* Hierarchy */
    GAMEOBJECT_HANDLE               hParent{};
    std::vector<GAMEOBJECT_HANDLE>  hChildren;

    /* Reset helper */
    void Reset()
    {
        bActive = false;
        layer = Layer::INVALID_LAYER;
        bPendingDestroy = false;
        iIndexInLayer = 0;
        std::fill(std::begin(iComponentSlots), std::end(iComponentSlots), 0);
        componentMask = 0;
        hParent = {};
        hChildren.clear();
    }

}GAMEOBJECT_DATA;

NS_END
