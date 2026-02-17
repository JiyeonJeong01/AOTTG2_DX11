#pragma once
#include <cstdint>
#include <vector>

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

NS_END
