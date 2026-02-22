#pragma once
#include <cstdint>
#include <vector>

#include "Engine_Enum.h"
#include "Engine_Typedef.h"
#include "Engine_Macro.h"

NS_BEGIN(Engine)

typedef struct tagComponentHandle
{
    uint32_t iHandle = 0;

    uint32_t Get_Index() const { return iHandle & Engine::Component::INDEX_MASK; }
    uint32_t Get_Version() const { return (iHandle & Component::VERSION_MASK) >> Component::VERSION_SHIFT; }

    static tagComponentHandle Create(uint32_t index, uint32_t version)
    {
        tagComponentHandle h;
        h.iHandle = (index & Component::INDEX_MASK) | ((version << Component::VERSION_SHIFT) & Component::VERSION_MASK);
        return h;
    }

    bool operator==(const tagComponentHandle& other) const { return iHandle == other.iHandle; }
    bool Is_Valid() const { return iHandle != 0; }
}COMPONENT_HANDLE;

static inline COMPONENT_HANDLE INVALID_HANDLE{ };
static inline uint32_t INVALID_HANDLE_UINT = { 0 };

typedef struct tagComponentGroup
{
    COMPONENT_HANDLE            tPrimary;
    std::vector<COMPONENT_HANDLE>    tExtras;
}COMPONENT_GROUP;

const std::array<PROCESSOR_ID, COMPONENT_MAX>
g_TypeToProcessorIndex =
{
    PROCESSOR_ID::TRANSFORM,        // TRANSFORM
    PROCESSOR_ID::PHYSICS,          // COLLIDER
    PROCESSOR_ID::PHYSICS,          // RIGIDBODY
    PROCESSOR_ID::SCRIPT,           // SCRIPT
    PROCESSOR_ID::MESH_RENDERER,    // MESH_RENDERER
    PROCESSOR_ID::ANIMATION,        // ANIMATOR
    PROCESSOR_ID::CAMERA,           // CAMERA
    PROCESSOR_ID::AUDIO,            // AUDIO_LISTENER
    PROCESSOR_ID::AUDIO,            // AUDIO_SOURCE

    PROCESSOR_ID::RECT_TRANSFORM,   // RECT_TRANSFORM
    PROCESSOR_ID::CANVAS_RENDERER,  // CANVAS_RENDERER

    PROCESSOR_ID::UI,               // UI_IMAGE
    PROCESSOR_ID::UI,               // UI_BUTTON
    PROCESSOR_ID::UI                // UI_TEXT
};

constexpr uint32_t COM_TO_INT(COMPONENT_TYPE eComType)
{
    return SCAST(uint32_t, eComType);
}

constexpr uint32_t PID_TO_INT(PROCESSOR_ID ePid)
{
    return SCAST(uint32_t, ePid);
}

constexpr uint32_t COM_TO_PID(COMPONENT_TYPE eComType)
{
    return PID_TO_INT(g_TypeToProcessorIndex[COM_TO_INT(eComType)]);
}

constexpr COMPONENT_TYPE INT_TO_COM(_int i)
{
    return SCAST(COMPONENT_TYPE, i);
}
NS_END
