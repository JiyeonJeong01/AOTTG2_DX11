#pragma once
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

static constexpr int32_t INVALID_ANIM_CLIP_INDEX = -1;

typedef struct tagAnimationClipIndexHasher
{
    size_t operator()(const uint64_t& key) const noexcept
    {
        uint32_t a = static_cast<uint32_t>(key >> 32);
        uint32_t b = static_cast<uint32_t>(key & 0xffffffffu);

        uint64_t hash = a;
        hash ^= static_cast<uint64_t>(b) + 0x9e3779b97f4a7c15ull + (hash << 6) + (hash >> 2);
        return static_cast<size_t>(hash);
    }
} ANIMATION_CLIP_INDEX_HASHER;

typedef struct ENGINE_DLL tagAnimatorData
{
    OBJECT_HANDLE       hObject{};
    _bool               bEnable = false;

    class CAnimator_Processor* pAnimator_Processor = nullptr;

    COMPONENT_HANDLE    hMeshRenderer = INVALID_HANDLE;
    uint32_t            iAnimationClip = INVALID_ANIM_CLIP_INDEX;
    uint32_t            iNextAnimationClip = INVALID_ANIM_CLIP_INDEX;

    _bool               bLoop = false;
    _bool               bPlaying = true;
    _bool               bIsBlending = false;

    _float              fTrackPosition = 0.f;
    _float              fPlaySpeed = 1.f;
    _float              fBlendElapsed = 0.f;
    _float              fBlendDuration = 0.02f;         /* 전부 기본 0.02초 보간 진행 */
        
    std::vector<uint32_t>   currentKeyFrameIndices;     /* 현재 애니메이션 클립에 대해 각 채널에 대응하는 키 프레임 인덱스 */

    /* local -> combined -> final -> [ SHADER ] */
    std::vector<_float4x4>   boneLocalMatrices;         /* 현재 프레임에서 각 Bone의 local transform 결과 */
    std::vector<_float4x4>   boneCombinedMatrices;      
    std::vector<_float4x4>   finalBoneMatrices;

    std::unordered_map<uint64_t, _float, ANIMATION_CLIP_INDEX_HASHER>   BlendMap;
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

    void                Set_NextAnimationClip(const std::string& strNextAnimClip);
    void                Set_NextAnimationClip(uint32_t iNextAnimClip);

    void                Reset_CurrentKeyFrameIndices();
    void                Reset_CurrentKeyFrameIndices(size_t iChannelCount);
};

NS_END

