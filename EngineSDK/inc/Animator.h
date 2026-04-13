#pragma once
#include "CComponent_Proxy_Base.h"
#include "Event.h"
#include "Animation_Event.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagAnimatorData
{
    OBJECT_HANDLE       hObject{};
    _bool               bEnable = false;

    class CAnimator_Processor* pAnimator_Processor = nullptr;

    COMPONENT_HANDLE    hMeshRenderer = INVALID_HANDLE;
    uint32_t            iAnimationClip = INVALID_ANIM_CLIP_INDEX;
    uint32_t            iNextAnimationClip = INVALID_ANIM_CLIP_INDEX;

    _bool               bPlaying = true;

    _float              fTrackPosition = 0.f;
    _float              fPlaySpeed = 1.f;
    _float              fBlendElapsed = 0.f;
    _float              fBlendDuration = 0.2f;         /* CURRENT VALUE!! 전부 기본 0.02초 보간 진행 */
        
    std::vector<uint32_t>   currentKeyFrameIndices;     /* 현재 애니메이션 클립에 대해 각 채널에 대응하는 키 프레임 인덱스 */

    /* local -> combined -> final -> [ SHADER ] */
    std::vector<_float4x4>   boneLocalMatrices;         /* 현재 프레임에서 각 Bone의 local transform 결과 */
    std::vector<_float4x4>   boneCombinedMatrices;      /* 누적된 뼈의 위치 */
    std::vector<_float4x4>   finalBoneMatrices;         /* 실제 스키닝에 쓰이는 행렬 = offsetMatrix[i] * currentCombinedMatrix[i]*/

    CEvent<const ANIMATION_EVENT_DATA&> OnAnimationFinished;
    CEvent<const ANIMATION_EVENT_DATA&> OnAnimationLooped;

    std::unordered_map<uint64_t, _float, ANIMATION_CLIP_INDEX_HASHER>   BlendMap;
    std::unordered_map<uint32_t, _bool>                                 LoopMap;
    std::unordered_map<std::string, uint32_t>                           NameToClipIndex;

} ANIMATOR_DATA;

class ENGINE_DLL CAnimator :  public CComponent_Proxy_Base<ANIMATOR_DATA, CAnimator, COMPONENT_TYPE::ANIMATOR>
{
public:
    CAnimator() : CComponent_Proxy_Base() {}
    CAnimator(DataType* pData, COMPONENT_HANDLE handle)
        : CComponent_Proxy_Base(pData, handle) {
    }
    ~CAnimator() override = default;


    void                Set_Loop(_bool bLoop, const std::string& strAnimClip);
    void                Set_Loop(_bool bLoop, uint32_t iAnimClip);

    _bool               Is_Playing() const;
    void                Set_Playing(_bool bPlaying);
    void                Play();
    void                Pause();
    void                Stop();

    _float              Get_TrackPosition() const;
    void                Set_TrackPosition(_float fTrackPosition);
    void                Add_TrackPosition(_float fDeltaTrackPosition);

    _float              Get_PlaySpeed() const;
    void                Set_PlaySpeed(_float fPlaySpeed);

    void                Set_NextAnimationClip(const std::string& strNextAnimClip);
    void                Set_NextAnimationClip(uint32_t iNextAnimClip);

    _uint               Get_CurAnimaionClipIdx() const;
    _uint               Get_AnimationClipIdx_By_Name(const std::string& strName) const;
    const std::string&  Get_Name_By_AnimationCliIdx(_uint iIdx) const;

    void                Reset_CurrentKeyFrameIndices();
    void                Reset_CurrentKeyFrameIndices(size_t iChannelCount);
};

NS_END

