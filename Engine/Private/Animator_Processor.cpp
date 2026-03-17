#include "Animator_Processor.h"

#include "Component_System.h"
#include "Component_Spec.h"

#include "Resource_System.h"
#include "Engine_Math.h"

#include "MeshRenderer_Processor.h"

CAnimator_Processor::CAnimator_Processor()
{
}

CAnimator_Processor::~CAnimator_Processor()
{
}

HRESULT CAnimator_Processor::Initialize()
{
    /* 팩토리 등록 */
    {
        SYS_COMPONENT.Register_InitialSpecFactory<CAnimator, ANIMATOR_SPEC>();
        SYS_COMPONENT.Register_BuildSpecFacotry<CAnimator>();
    }

    return S_OK;
}

void CAnimator_Processor::Update(_float fDT)
{
    const auto& pages = m_Pool.GetPages();

    for (const auto& upPage : pages)
    {
        auto* pPage = upPage.get();
        if (!pPage) continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            ANIMATOR_DATA* pData = pPage->Get_Ptr(i);
            if (!pData) continue;
            if (!pData->bEnable) continue;

            Update_Animator(pData, fDT);
        }
    }
}

void CAnimator_Processor::LateUpdate(_float fDT)
{

}

HRESULT CAnimator_Processor::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec)
{
    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CAnimator_Processor::Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    return nullptr;
}

void CAnimator_Processor::Initialize_Component_Data(COMPONENT_HANDLE hComponent)
{
    auto pData = m_Pool.Get_Data_By_Handle(hComponent);

    CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
    IF_NULL_RETURN_MSG_BREAK(pObj, , "pObj is nullptr");

    CMeshRenderer mr = pObj->Get_Component<CMeshRenderer>();
    pData->hMeshRenderer = mr.Get_Handle();
    mr->hAnimator = hComponent;

    pData->bPlaying = true;
}

void CAnimator_Processor::Reset_Data_On_Deallocate(COMPONENT_HANDLE hScript, ANIMATOR_DATA* pData)
{
}
 
/* 컴포넌트 하나에 대한 전체 업데이트 진입점 */
void CAnimator_Processor::Update_Animator(ANIMATOR_DATA* pData, _float fDT)
{
    IF_NULL_RETURN_MSG_BREAK(pData, , "pData is nullptr");

    MESH_RENDERER_DATA* pMeshRenderer = nullptr;
    MODEL_ENTRY* pModel = nullptr;


    if (!Resolve_Model_Entry(pData, pMeshRenderer, pModel))
        return;

    ANIMATION_CLIP_ENTRY* pClip = Resolve_Current_AnimationClip(pData, pModel);
    if (!pClip)
        return;

    if (!pModel->Has_Skeleton())
        return;

    Ensure_RuntimeBuffers(pData, *pModel, *pClip);

    _bool bFinished = false;
    Update_TrackPosition(pData, *pClip, fDT, bFinished);

    Evaluate_AnimationChannels(pData, pModel->tSkeleton, *pClip);
    Build_BoneCombinedMatrices(pData, pModel->tSkeleton);
    Build_FinalBoneMatrices(pData, *pModel);
}

void CAnimator_Processor::Update_TrackPosition(ANIMATOR_DATA* pData, const ANIMATION_CLIP_ENTRY& tClip, _float fDT, _bool& bOutFinished)
{
    bOutFinished = false;

    if (!pData->bPlaying) return;

    const _float fTickPerSecond = (tClip.fTickPerSecond <= 0.f) ? 1.f : tClip.fTickPerSecond;
    pData->fTrackPosition += fDT * fTickPerSecond * pData->fPlaySpeed;

    /* 애니메이션 재생이 끝난 경우 */
    if (pData->fTrackPosition >= tClip.fDuration)
    {
        if (!pData->bLoop) /* 반복 재생하지 않는 경우  */
        {
            pData->fTrackPosition = tClip.fDuration;
            pData->bPlaying = false;
            bOutFinished = true;
            return;
        }

        pData->fTrackPosition = fmodf(pData->fTrackPosition, tClip.fDuration);

        for (uint32_t& iKeyFrameIndex : pData->currentKeyFrameIndices) /* 반복 재생 */
            iKeyFrameIndex = 0;
    }
}

void CAnimator_Processor::Ensure_RuntimeBuffers(ANIMATOR_DATA* pData, const MODEL_ENTRY& tModel, const ANIMATION_CLIP_ENTRY& tClip)
{
    const size_t iBoneCount = tModel.tSkeleton.bones.size();
    const size_t iChannelCount = tClip.channels.size();

    /* 각 채널에 대한 키프레임 인덱스 */
    if (pData->currentKeyFrameIndices.size() != iChannelCount)
        pData->currentKeyFrameIndices.assign(iChannelCount, 0);

    /* 각 뼈에 대한 로컬, 컴바인드, 파이널 행렬 */
    if (pData->boneLocalMatrices.size() != iBoneCount)
        pData->boneLocalMatrices.assign(iBoneCount, Math::Identity());

    if (pData->boneCombinedMatrices.size() != iBoneCount)
        pData->boneCombinedMatrices.assign(iBoneCount, Math::Identity());

    if (pData->finalBoneMatrices.size() != iBoneCount)
        pData->finalBoneMatrices.assign(iBoneCount, Math::Identity());
}

/* model인지 아닌지 확인 */
_bool CAnimator_Processor::Resolve_Model_Entry(const ANIMATOR_DATA* pData, MESH_RENDERER_DATA*& pOutMeshRenderer, MODEL_ENTRY*& pOutModel)
{
    pOutMeshRenderer = nullptr;
    pOutModel = nullptr;

    if (!pData->hMeshRenderer.Is_Valid())
        return false;

    pOutMeshRenderer = SYS_COMPONENT.Get_Proxy<CMeshRenderer>(COMPONENT_TYPE::MESH_RENDERER, pData->hMeshRenderer)._Data();
    IF_NULL_RETURN_MSG_BREAK(pOutMeshRenderer, false, "Can't find mesh renderer data");

    if (pOutMeshRenderer->hMesh == INVALID_HANDLE_UINT)
        return false;

    if (!SYS_RESOURCE.Is_ModelHandle(pOutMeshRenderer->hMesh))
        return false;

    pOutModel = SYS_RESOURCE.Get_Model(pOutMeshRenderer->hMesh);
    if (!pOutModel)
        return false;

    return true;
}

/* hAnimationclip 값을 이용하여 MODEL_ENTRY의 vecAnimClips에서 현재 클립을 가져온다. */
ANIMATION_CLIP_ENTRY* CAnimator_Processor::Resolve_Current_AnimationClip(const ANIMATOR_DATA* pData, MODEL_ENTRY* pModel)
{
    IF_NULL_RETURN_MSG_BREAK(pModel,nullptr, "Can't find mesh renderer data");

    if (pModel->vecAnimClips.empty())
        return nullptr;

    const uint32_t iClipIndex = pData->iAnimationClip;
    if (iClipIndex >= pModel->vecAnimClips.size())
        return nullptr;

    return &pModel->vecAnimClips[iClipIndex];
}

void CAnimator_Processor::Evaluate_AnimationChannels(ANIMATOR_DATA* pData, const SKELETON_ENTRY& tSkeleton, const ANIMATION_CLIP_ENTRY& tClip)
{
    /* 일단 모든 Bone을 기본 자세로 채운다. */
    /* 애니메이션 클립이 모든 Bone을 전부 채널로 갖고 있지 않을 수 있기 때문에 쓰레기값 방지 */
    const size_t iBoneCount = tSkeleton.bones.size();
    for (size_t i = 0; i < iBoneCount; ++i)
    {
        if (i < pData->boneLocalMatrices.size())
            pData->boneLocalMatrices[i] = tSkeleton.bones[i].matLocalBind;
    }

    /* 실제로 움직이는 본들에 대한 정보 : 애니메이션 채널이 있는 Bone만 계산하여 그 값을 덮어 쓴다. */
    /* 애니메이션 클립 안에 들어 있는 특정 본 하나의 애니메이션 정보 (e.g., 걷기 애니메이션의 오른쪽 정강이 애니메이션) */
    for (size_t i = 0; i < tClip.channels.size(); ++i)
    {
        const ANIMATION_CHANNEL_ENTRY& tChannel = tClip.channels[i];                /* 오른쪽 정강이 채널(=Bone)*/
        if (tChannel.iBoneIndex < 0)
            continue;

        if (To<size_t>(tChannel.iBoneIndex) >= pData->boneLocalMatrices.size())     /* 해당 채널이 해당하는 BoneIndex의 로컬 행렬 */
            continue;

        _float4x4 matLocal{};
        Evaluate_Channel_LocalTransform(pData, tChannel, (_uint)i, matLocal);
        pData->boneLocalMatrices[tChannel.iBoneIndex] = matLocal;                   
    }
}

/* 채널 별 처리 */
void CAnimator_Processor::Evaluate_Channel_LocalTransform(ANIMATOR_DATA* pData, const ANIMATION_CHANNEL_ENTRY& tChannel, _uint iChannelIndex, _float4x4& outLocalMatrix)
{
    outLocalMatrix = Math::Identity();

    /* 채널이 비었거나, 채널의 인덱스가 없는 경우 */
    if (tChannel.vecKeyFrames.empty() || iChannelIndex >= pData->currentKeyFrameIndices.size())
        return;

    if (tChannel.vecKeyFrames.size() == 1)
    {
        /* 해당 키프레임에 맞는 애니메이션 행렬 만들기 */
        Math::Store(outLocalMatrix, Make_AffineMatrix(tChannel.vecKeyFrames[0]));
        return;
    }

    /* 현재 애니메이션이 어느 키프레임까지 진행됐는지  */
    _uint& iCurrentKeyFrameIndex = pData->currentKeyFrameIndices[iChannelIndex];
    if (pData->fTrackPosition <= 0.f)
        iCurrentKeyFrameIndex = 0;

    const auto& vecKeyFrames = tChannel.vecKeyFrames;
    const ANIM_KEYFRAME& tLast = vecKeyFrames.back();

    /* 현재 애니메이션을 끝까지 재생한 상태 */
    if (pData->fTrackPosition >= tLast.fTrackPosition)
    {
        Math::Store(outLocalMatrix, Make_AffineMatrix(tLast));
        return;
    }


    while ((iCurrentKeyFrameIndex + 1) < vecKeyFrames.size()
        && pData->fTrackPosition >= vecKeyFrames[iCurrentKeyFrameIndex + 1].fTrackPosition)
        ++iCurrentKeyFrameIndex;

    if ((iCurrentKeyFrameIndex + 1) >= vecKeyFrames.size())
    {
        Math::Store(outLocalMatrix, Make_AffineMatrix(vecKeyFrames.back()));
        return;
    }

    /* 보간 시작 */
    const ANIM_KEYFRAME& tLeft = vecKeyFrames[iCurrentKeyFrameIndex];
    const ANIM_KEYFRAME& tRight = vecKeyFrames[iCurrentKeyFrameIndex + 1];

    const _float fRatio = Compute_KeyFrameRatio(tLeft, tRight, pData->fTrackPosition);
    Math::Store(outLocalMatrix, Make_AffineMatrix(tLeft, tRight, fRatio));
}

/* 부모 Bone을 따라 행렬 누적 */
void CAnimator_Processor::Build_BoneCombinedMatrices(ANIMATOR_DATA* pData, const SKELETON_ENTRY& tSkeleton)
{
    const size_t iBoneCount = tSkeleton.bones.size();
    if (iBoneCount == 0)
        return;

    for (size_t i = 0; i < iBoneCount; ++i)
    {
        const BONE_ENTRY& tBone = tSkeleton.bones[i];
        const _matrix matLocal = Math::Load(pData->boneLocalMatrices[i]);

        if (tBone.iParentBoneIndex < 0)
        {
            Math::Store(pData->boneCombinedMatrices[i], matLocal);
        }
        else /* 부모의 Combined 행렬 가져오기 */
        {
            const _matrix matParentCombined = Math::Load(pData->boneCombinedMatrices[tBone.iParentBoneIndex]);
            Math::Store(pData->boneCombinedMatrices[i], matLocal * matParentCombined);
        }
    }
}

/* 최종적으로 셰이더에 넘길 행렬 생성 */
void CAnimator_Processor::Build_FinalBoneMatrices(ANIMATOR_DATA* pData, const MODEL_ENTRY& tModel)
{
    const size_t iBoneCount = tModel.tSkeleton.bones.size();
    if (iBoneCount == 0)
        return;

    for (size_t i = 0; i < iBoneCount; ++i)
    {
        const BONE_ENTRY& tBone = tModel.tSkeleton.bones[i];

        const _matrix matOffset = XMLoadFloat4x4(&tBone.matOffset);
        const _matrix matCombined = XMLoadFloat4x4(&pData->boneCombinedMatrices[i]);

        /* TODO : 필요 시 root inverse / PreTransform 보정 추가 */
        XMStoreFloat4x4(&pData->finalBoneMatrices[i], matOffset * matCombined);
    }
}

_float CAnimator_Processor::Compute_KeyFrameRatio(const ANIM_KEYFRAME& tLeft, const ANIM_KEYFRAME& tRight, _float fTrackPosition) const
{
    /* 키프레임에 따른 애니메이션 보간 비율 결정 */
    const _float fDenom = tRight.fTrackPosition - tLeft.fTrackPosition;
    if (fDenom <= 0.f)
        return 0.f;

    const _float fRatio = (fTrackPosition - tLeft.fTrackPosition) / fDenom;
    return std::clamp(fRatio, 0.f, 1.f);
}

_matrix CAnimator_Processor::Make_AffineMatrix(const ANIM_KEYFRAME& tLeft, const ANIM_KEYFRAME& tRight, _float fRatio) const
{
    /* 보간된 비율에 따라 행렬 생성하기 */
    const _vector vLeftScale = Math::Load(tLeft.vScale);
    const _vector vRightScale = Math::Load(tRight.vScale);
    const _vector vScale = XMVectorLerp(vLeftScale, vRightScale, fRatio);

    const _vector vLeftRotation = Math::Load(tLeft.vRotation);
    const _vector vRightRotation = Math::Load(tRight.vRotation);
    const _vector vRotation = XMQuaternionSlerp(vLeftRotation, vRightRotation, fRatio);

    const _vector vLeftTranslation = XMVectorSetW(XMLoadFloat3(&tLeft.vTranslation), 1.f);
    const _vector vRightTranslation = XMVectorSetW(XMLoadFloat3(&tRight.vTranslation), 1.f);
    const _vector vTranslation = XMVectorLerp(vLeftTranslation, vRightTranslation, fRatio);

    return XMMatrixAffineTransformation(
        vScale,
        XMVectorSet(0.f, 0.f, 0.f, 1.f),
        vRotation,
        vTranslation);
}

_matrix CAnimator_Processor::Make_AffineMatrix(const ANIM_KEYFRAME& tKeyFrame) const
{
    /* 키 프레임에 따른 행렬 만들기 */
    const _vector vScale = XMLoadFloat3(&tKeyFrame.vScale);
    const _vector vRotation = XMLoadFloat4(&tKeyFrame.vRotation);
    const _vector vTranslation = XMVectorSetW(XMLoadFloat3(&tKeyFrame.vTranslation), 1.f);

    return XMMatrixAffineTransformation(
        vScale,
        XMVectorSet(0.f, 0.f, 0.f, 1.f),
        vRotation,
        vTranslation);
}

std::unique_ptr<CAnimator_Processor> CAnimator_Processor::Create()
{
    auto pInstance = std::make_unique<CAnimator_Processor>();

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}
