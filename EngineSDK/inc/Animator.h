#pragma once
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagAnimatorData
{
    OBJECT_HANDLE       hObject{};
    _bool               bEnable = false;

    COMPONENT_HANDLE    hMeshRenderer = INVALID_HANDLE;
    uint32_t            iAnimationClip = INVALID_HANDLE_UINT;

    _bool               bLoop = false;
    _bool               bPlaying = true;

    _float              fTrackPosition = 0.f;
    _float              fPlaySpeed = 1.f;
        
    std::vector<uint32_t>   currentKeyFrameIndices;     /* 각 채널에 대응하는 키 프레임 인덱스 */
    std::vector<_float4x4>   boneLocalMatrices;         /* 현재 프레임에서 각 Bone의 local transform 결과 */
    std::vector<_float4x4>   boneCombinedMatrices;
    std::vector<_float4x4>   finalBoneMatrices;

} ANIMATOR_DATA;

class ENGINE_DLL CAnimator :  public CComponent_Proxy_Base<ANIMATOR_DATA, CAnimator, COMPONENT_TYPE::ANIMATOR>
{

public:
    CAnimator() : CComponent_Proxy_Base() {}
    CAnimator(DataType* pData, COMPONENT_HANDLE handle)
        : CComponent_Proxy_Base(pData, handle) {
    }
    ~CAnimator() override = default;


    void                Set_Loop(_bool bLoop);

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

    void                Reset_CurrentKeyFrameIndices();
    void                Reset_CurrentKeyFrameIndices(size_t iChannelCount);
};

NS_END

