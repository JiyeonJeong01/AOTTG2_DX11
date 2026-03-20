#pragma once

#include "Engine_Define.h"
#include "Identity.h"

typedef struct ENGINE_DLL tagAnimKeyFrame
{
    _float      fTrackPosition = 0.f;

    _float3     vScale = { 1.f, 1.f, 1.f };
    _float4     vRotation = { 0.f, 0.f, 0.f, 1.f };
    _float3     vTranslation = { 0.f, 0.f, 0.f };
}ANIM_KEYFRAME;

typedef struct ENGINE_DLL tagAnimationChannelEntry
{
    std::string         strBoneName = "";
    int32_t             iBoneIndex = -1;

    std::vector<ANIM_KEYFRAME>  vecKeyFrames;
} ANIMATION_CHANNEL_ENTRY;

typedef struct ENGINE_DLL tagAnimationClipEntry
{
    ASSET_GUID          tGUID{};
    std::string         strName = "";

    _float              fDuration = 0.f;
    _float              fTickPerSecond = 0.f;

    std::vector<ANIMATION_CHANNEL_ENTRY> channels;

    const ANIMATION_CHANNEL_ENTRY* Find_Channel_ByBoneIndex(int32_t iBoneIndex) const
    {
        for (const ANIMATION_CHANNEL_ENTRY& tChannel : channels)
        {
            if (tChannel.iBoneIndex == iBoneIndex)
                return &tChannel;
        }

        return nullptr;
    }

} ANIMATION_CLIP_ENTRY;
