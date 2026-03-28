#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagAnimationEventData : public EVENT_DATA
{
public:
    tagAnimationEventData(uint32_t iAnimClip, const std::string& strAnim)
        : EVENT_DATA(EVENT_TYPE::ANIMATION), iAnimationClip(iAnimClip), strAnimName(strAnim) {
    };
    ~tagAnimationEventData() override = default;

    uint32_t            iAnimationClip = INVALID_ANIM_CLIP_INDEX;
    std::string         strAnimName = "";
}ANIMATION_EVENT_DATA;

NS_END
