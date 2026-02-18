#pragma once

#include <cstdint>
#include <vector>

#include "Engine_Enum.h"
#include "Engine_Macro.h"
#include "Engine_Typedef.h"
#include "Identity.h"

NS_BEGIN(Engine)

typedef struct tagGameObjectData
{
    uint32_t    iVersion = 1;       /* slot's current version */
    _bool       bActive = false;
    _bool       bPendingDestroy = false;
    INSTANCE_UUID   tUUID{};

    /* For layer access in O(1) */
    Layer::LAYER_ID layer = Layer::INVALID_LAYER;
    uint32_t iIndexInLayer = 0;

    /* Components */
    uint32_t iComponentSlots[COMPONENT_MAX] = { 0, };
    Component::COMPONENT_MASK   componentMask = 0;

    /* Hierarchy */
    OBJECT_HANDLE               hParent{};
    std::vector<OBJECT_HANDLE>  hChildren;

    /* Reset helper */
    void Reset()
    {
        bActive = false;
        layer = Layer::INVALID_LAYER;
        tUUID = {};
        bPendingDestroy = false;
        iIndexInLayer = 0;
        std::fill(std::begin(iComponentSlots), std::end(iComponentSlots), 0);
        componentMask = 0;
        hParent = {};
        hChildren.clear();
    }

}GAMEOBJECT_DATA;

typedef struct tagUIObjectData
{
    uint32_t        iVersion = 1;

    _bool           bActive = false;
    _bool           bPendingDestroy = false;
    INSTANCE_UUID   tUUID{};

    // ---- UI 전용 ----
    uint32_t        iSortOrder = 0;
    _bool           bVisible = true;
    _bool           bInteractable = true;
    _bool           bRaycastTarget = true;

    // RectTransform 대체 (최소 버전)
    // (너 엔진 타입에 맞춰 _float2/_float4 등으로 바꿔)
    _float2         anchorMin{ 0.f, 0.f };
    _float2         anchorMax{ 1.f, 1.f };
    _float2         pivot{ 0.5f, 0.5f };
    _float2         anchoredPos{ 0.f, 0.f };
    _float2         sizeDelta{ 0.f, 0.f };
    _float2         scale{ 1.f, 1.f };
    float           rotation = 0.f;        // 2D면 z-rot만

    // ---- Components ----
    uint32_t                    iComponentSlots[COMPONENT_MAX] = { 0, };
    Component::COMPONENT_MASK   componentMask = 0;

    // ---- UI Hierarchy ----
    OBJECT_HANDLE               hParent{};
    std::vector<OBJECT_HANDLE>  hChildren{};

    void Reset()
    {
        bActive = false;
        bPendingDestroy = false;
        tUUID = {};

        iSortOrder = 0;
        bVisible = true;
        bInteractable = true;
        bRaycastTarget = true;

        anchorMin = { 0.f, 0.f };
        anchorMax = { 1.f, 1.f };
        pivot = { 0.5f, 0.5f };
        anchoredPos = { 0.f, 0.f };
        sizeDelta = { 0.f, 0.f };
        scale = { 1.f, 1.f };
        rotation = 0.f;

        std::fill(std::begin(iComponentSlots), std::end(iComponentSlots), 0);
        componentMask = 0;

        hParent = {};
        hChildren.clear();
    }

} UIOBJECT_DATA;
static constexpr INSTANCE_UUID DUMMY_UUID{};

NS_END
