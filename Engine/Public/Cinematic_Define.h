#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

enum class CINEMATIC_SHOT_TYPE : uint8_t
{
    CUT,
    BLEND,
    END
};

enum class CINEMATIC_EASE : uint8_t
{
    LINEAR,
    EASE_IN,
    EASE_OUT,
    EASE_IN_OUT,
    END
};

enum class CINEMATIC_EVENT_TYPE : uint8_t
{
    NONE,
    CUSTOM,
    START,
    FINISH,
    END
};

typedef struct ENGINE_DLL tagCinematicCameraKey
{
    _float      fTime = 0.f;

    _float3     vPosition{ 0.f, 0.f, 0.f };
    _float4     vRotationQuat{ 0.f, 0.f, 0.f, 1.f };

    _float      fFovy = 60.f;

    CINEMATIC_EASE eEase = CINEMATIC_EASE::LINEAR;

} CINEMATIC_CAMERA_KEY;

typedef struct ENGINE_DLL tagCinematicEventKey
{
    _float                  fTime = 0.f;
    CINEMATIC_EVENT_TYPE    eType = CINEMATIC_EVENT_TYPE::NONE;

    char                    szEventName[64] = {};

} CINEMATIC_EVENT_KEY;

typedef struct ENGINE_DLL tagCinematicShakeKey
{
    _float      fStartTime = 0.f;
    _float      fEndTime = 0.f;

    _float      fAmplitudePos = 0.f;
    _float      fAmplitudeRot = 0.f;
    _float      fFrequency = 0.f;

} CINEMATIC_SHAKE_KEY;

typedef struct ENGINE_DLL tagCinematicShotKey
{
    _float                  fTime = 0.f;

    uint32_t                iCameraKeyIndex = 0;
    _float                  fBlendDuration = 0.f;

    CINEMATIC_SHOT_TYPE     eType = CINEMATIC_SHOT_TYPE::CUT;

} CINEMATIC_SHOT_KEY;

typedef struct ENGINE_DLL tagCinematicClip
{
    char                                szName[64] = {};
    _float                              fDuration = 0.f;

    std::vector<CINEMATIC_CAMERA_KEY>   vecCameraKeys;
    std::vector<CINEMATIC_EVENT_KEY>    vecEventKeys;
    std::vector<CINEMATIC_SHAKE_KEY>    vecShakeKeys;
    std::vector<CINEMATIC_SHOT_KEY>     vecShotKeys;

} CINEMATIC_CLIP;

typedef struct ENGINE_DLL tagCinematicFileHeader
{
    uint32_t    iMagic = 'CMTK';
    uint32_t    iVersion = 1;

    uint32_t    iCameraKeyCount = 0;
    uint32_t    iEventKeyCount = 0;
    uint32_t    iShakeKeyCount = 0;
    uint32_t    iShotKeyCount = 0;

} CINEMATIC_FILE_HEADER;




NS_END
