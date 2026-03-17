#pragma once
#include "Render_Struct.h"
#include "Animator.h"
#include "Skeleton.h"
#include "Animation_Clip.h"
#include "MeshRenderer.h"
#include "Model.h"
#include "Component_Processor_Impl.h"

NS_BEGIN(Engine)

class ENGINE_DLL CAnimator_Processor final : public CComponent_Processor_Impl<CAnimator, COMPONENT_TYPE::ANIMATOR>
{
    DEF_PROCESSOR_ID(PROCESSOR_ID::ANIMATION)
public:
    CAnimator_Processor();
    ~CAnimator_Processor() override;
public:
    HRESULT Initialize() override;
    void Update(_float fDT) override;
    void LateUpdate(_float fDT) override;

    HRESULT Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec) override;
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;

private:
    void Initialize_Component_Data(COMPONENT_HANDLE hComponent) override;
    void Reset_Data_On_Deallocate(COMPONENT_HANDLE hScript, ANIMATOR_DATA* pData);

private :
    void Update_Animator(ANIMATOR_DATA* pData, _float fDT);
    void Update_TrackPosition(ANIMATOR_DATA* pData, const ANIMATION_CLIP_ENTRY& tClip, _float fDT, _bool& bOutFinished);
    void Ensure_RuntimeBuffers(ANIMATOR_DATA* pData, const MODEL_ENTRY& tModel, const ANIMATION_CLIP_ENTRY& tClip);

    _bool Resolve_Model_Entry(const ANIMATOR_DATA* pData, MESH_RENDERER_DATA*& pOutMeshRenderer, MODEL_ENTRY*& pOutModel);
    ANIMATION_CLIP_ENTRY* Resolve_Current_AnimationClip(const ANIMATOR_DATA* pData, MODEL_ENTRY* pModel);

    void Evaluate_AnimationChannels(ANIMATOR_DATA* pData, const SKELETON_ENTRY& tSkeleton, const ANIMATION_CLIP_ENTRY& tClip);
    void Evaluate_Channel_LocalTransform(
        ANIMATOR_DATA* pData,
        const ANIMATION_CHANNEL_ENTRY& tChannel,
        _uint iChannelIndex,
        _float4x4& outLocalMatrix);

    void Build_BoneCombinedMatrices(ANIMATOR_DATA* pData, const SKELETON_ENTRY& tSkeleton);
    void Build_FinalBoneMatrices(ANIMATOR_DATA* pData, const MODEL_ENTRY& tModel);

    _float Compute_KeyFrameRatio(const ANIM_KEYFRAME& tLeft, const ANIM_KEYFRAME& tRight, _float fTrackPosition) const;
    _matrix Make_AffineMatrix(const ANIM_KEYFRAME& tLeft, const ANIM_KEYFRAME& tRight, _float fRatio) const;
    _matrix Make_AffineMatrix(const ANIM_KEYFRAME& tKeyFrame) const;


public:
    static std::unique_ptr<CAnimator_Processor> Create();
};

NS_END
