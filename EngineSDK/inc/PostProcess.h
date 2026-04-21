#pragma once

#include "Engine_Define.h"

NS_BEGIN(Engine)

typedef struct tagFogDesc
{
    _bool   bEnable = true;
    _float3 vColor = { 0.62f, 0.66f, 0.72f };
    _float  fStart = 0.2f;
    _float  fEnd = 90.f;
    _float  fDensity = 0.03f;
    _float  fPadding[3] = {};
} FOG_DESC;

typedef struct tagVignetteDesc
{
    _bool   bEnable = false;
    _float  fIntensity = 0.5f;
    _float  fPower = 2.f;
    _float  fRoundness = 1.f;
} VIGNETTE_DESC;

typedef struct tagBlurDesc
{
    _bool   bEnable = false;
    _float  fStrength = 0.f;
    _float  fPadding[2] = {};
} BLUR_DESC;

typedef struct tagPostProcessDesc
{
    FOG_DESC        tFog;
    VIGNETTE_DESC   tVignette;
    BLUR_DESC       tBlur;
} POST_PROCESS_DESC;

typedef struct tagPostProcessParam
{
    _float4 vFogColor = { 0.7f, 0.7f, 0.8f, 1.f };
    _float4 vFogParams = { 3.f, 80.f, 0.012f, 0.f }; // x=start, y=end, z=density, w=enable

    _float4 vVignetteParams = { 0.5f, 2.f, 1.f, 1.f }; // x=intensity, y=power, z=roundness, w=enable
    _float4 vBlurParams = { 0.f, 0.f, 0.f, 0.f };      // x=strength, y=enable
} POST_PROCESS_PARAM;

NS_END
