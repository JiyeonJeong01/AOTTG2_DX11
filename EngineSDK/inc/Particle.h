#pragma once

#include "Engine_Define.h"
#include "Identity.h"
#include "Component_Struct.h"
#include "Render_Struct.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagParticleDesc
{
    ASSET_GUID      tTextureGUID{};

    _uint           iMaxParticles = 0;

    /* --- elapsed, total --- */
    _float2         vLifeTime{ 1.f, 1.f };
    /* --- min, max --- */
    _float2         vSpeed{ 1.f, 1.f };
    _float2         vScale{ 1.f, 1.f };

    _float3         vCenter{};
    _float3         vRange{};
    _float3         vPivot{};

    _bool           isLoop = true;
    PARTICLE_SIMULATION eSimulation = PARTICLE_SIMULATION::DROP;
} PARTICLE_DESC;

typedef struct ENGINE_DLL tagParticleEntry
{
    ASSET_GUID      tGUID{};
    ASSET_GUID      tTextureGUID{};

    uint32_t        hTexture = INVALID_HANDLE_UINT;

    _uint           iMaxParticles = 0;

    _float2         vLifeTime{ 1.f, 1.f };
    _float2         vSpeed{ 1.f, 1.f };
    _float2         vScale{ 1.f, 1.f };

    _float3         vCenter{};
    _float3         vRange{};
    _float3         vPivot{};

    _bool           isLoop = true;
    PARTICLE_SIMULATION eSimulation = PARTICLE_SIMULATION::DROP;

    _bool Is_Valid() const noexcept
    {
        return iMaxParticles > 0;
    }
} PARTICLE_ENTRY;

NS_END
