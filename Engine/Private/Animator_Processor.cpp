#include "Animator_Processor.h"

#include "Component_System.h"
#include "Component_Spec.h"

#include "Resource_System.h"
#include "Engine_Math.h"

#include "MeshRenderer_Processor.h"
#include "Component_System.h"

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

    m_pMeshRenderer_Processor = SYS_COMPONENT.Bind_Processor<CMeshRenderer_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pMeshRenderer_Processor, E_FAIL, "processor is nullptr");

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
    auto pData = m_Pool.Get_Data_By_Handle(handle);
    IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "pData is nullptr");

    const ANIMATOR_SPEC* pAnimatorSpec = dynamic_cast<const ANIMATOR_SPEC*>(pSpec);
    IF_NULL_RETURN_MSG_BREAK(pAnimatorSpec, E_FAIL, "pAnimatorSpec is nullptr");

    pData->bEnable = pAnimatorSpec->bEnable;
    pData->bPlaying = pAnimatorSpec->bPlaying;
    pData->iAnimationClip = pAnimatorSpec->iAnimationClip;
    pData->fPlaySpeed = pAnimatorSpec->fPlaySpeed;
    pData->fBlendDuration = pAnimatorSpec->fBlendDuration;
    pData->BlendMap = pAnimatorSpec->BlendMap;
    pData->LoopMap = pAnimatorSpec->LoopMap;

    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CAnimator_Processor::Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    auto pData = m_Pool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, nullptr, "pData is nullptr");

    auto pSpec = std::make_unique<ANIMATOR_SPEC>();

    pSpec->bEnable = pData->bEnable;
    pSpec->bPlaying = pData->bPlaying;
    pSpec->iAnimationClip = pData->iAnimationClip;
    pSpec->fPlaySpeed = pData->fPlaySpeed;
    pSpec->fBlendDuration = pData->fBlendDuration;
    pSpec->BlendMap = pData->BlendMap;
    pSpec->LoopMap = pData->LoopMap;

    return pSpec;
}

void CAnimator_Processor::Initialize_Component_Data(COMPONENT_HANDLE hComponent)
{
    auto pData = m_Pool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, , "pData is nullptr");

    CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
    IF_NULL_RETURN_MSG_BREAK(pObj, , "pObj is nullptr");

    CMeshRenderer mr = pObj->Get_Component<CMeshRenderer>();
    IF_TRUE_RETURN_MSG_BREAK(!mr.Is_Valid(), , "mesh renderer is invalid");
    mr->hAnimator = hComponent; /* 이거 진짜 필수 빠지면 안그려짐 ㅜㅜ */

    pData->hMeshRenderer = mr.Get_Handle();
    pData->pAnimator_Processor = this;
    pData->NameToClipIndex.clear();
    pData->bPlaying = true;
    pData->iNextAnimationClip = INVALID_ANIM_CLIP_INDEX;
    pData->fTrackPosition = 0.f;
    pData->fBlendElapsed = 0.f;
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

    /* 애니메이터 컴포넌트가 현재 재생 중인 애니메이션 클립 */
    ANIMATION_CLIP_ENTRY* pCurClip = Resolve_Current_AnimationClip(pData, pModel);
    if (!pCurClip)
        return;

    if (!pModel->Has_Skeleton())
        return;

    Ensure_RuntimeBuffers(pData, *pModel, *pCurClip);

    _bool bFinished = false;
    const uint32_t iFinishedClip = pData->iAnimationClip;
    const std::string strFinishedClipName = pCurClip->strName;

    Update_TrackPosition(pData, *pCurClip, fDT, bFinished);
    Update_BlendState(pData, fDT);

    if (pData->iNextAnimationClip != INVALID_ANIM_CLIP_INDEX
        && pData->fBlendElapsed >= pData->fBlendDuration)
    {
        ANIMATION_CLIP_ENTRY* pNextClip = Resolve_Next_AnimationClip(pData, pModel);
        if (pNextClip)
        {
            pData->iAnimationClip = pData->iNextAnimationClip;
            pData->fTrackPosition = Get_NextClipTrackPosition(pData, *pNextClip);
            pData->currentKeyFrameIndices.assign(pNextClip->channels.size(), 0);
            pData->bPlaying = true;
            pCurClip = pNextClip;

            Ensure_RuntimeBuffers(pData, *pModel, *pCurClip);
        }

        pData->iNextAnimationClip = INVALID_ANIM_CLIP_INDEX;
        pData->fBlendElapsed = 0.f;
        pData->fBlendDuration = 0.f;
    }

    Evaluate_AnimationChannels(pData, *pModel, *pCurClip);
    Build_BoneCombinedMatrices(pData, pModel->tSkeleton);

    /* 매 프레임 각 Bone이 어디로 움직였는지 finalBoneMatrices에 계산해둔다. */
    Build_FinalBoneMatrices(pData, *pModel);

    if (bFinished)
    {
        ANIMATION_EVENT_DATA tEventData{ iFinishedClip, strFinishedClipName };
        pData->OnAnimationFinished.Invoke(tEventData);
    }
}

void CAnimator_Processor::Update_TrackPosition(ANIMATOR_DATA* pData, const ANIMATION_CLIP_ENTRY& tClip, _float fDT, _bool& bOutFinished)
{
    bOutFinished = false;

    if (!pData->bPlaying)
        return;

    /* 초당 누적되는 틱 */
    const _float fTickPerSecond = (tClip.fTickPerSecond <= 0.f) ? 1.f : tClip.fTickPerSecond;
    pData->fTrackPosition += fDT * fTickPerSecond * pData->fPlaySpeed;

    /* 애니메이션 재생이 끝난 경우 */
    if (pData->fTrackPosition >= tClip.fDuration)
    {
        const _bool bLoop = Is_LoopClip(pData, pData->iAnimationClip);

        if (!bLoop)
        {
            pData->fTrackPosition = tClip.fDuration;
            pData->bPlaying = false;
            bOutFinished = true;
            return;
        }

        ANIMATION_EVENT_DATA tEventData{ pData->iAnimationClip, tClip.strName };

        /* 이벤트 발생 */
        pData->OnAnimationLooped.Invoke(tEventData);

        /* 초과 시간 버리지 않고 유지 */
        pData->fTrackPosition = fmodf(pData->fTrackPosition, tClip.fDuration);

        for (uint32_t& iKeyFrameIndex : pData->currentKeyFrameIndices)
            iKeyFrameIndex = 0;
    }
}

void CAnimator_Processor::Update_BlendState(ANIMATOR_DATA* pData, _float fDT)
{
    if (pData->iNextAnimationClip == INVALID_ANIM_CLIP_INDEX)
        return;

    pData->fBlendElapsed += fDT;

    if (pData->fBlendElapsed > pData->fBlendDuration)
        pData->fBlendElapsed = pData->fBlendDuration;
}

/* 인덱스 범위 보장 등 에러 방지 */
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

/* MeshRenderer로 MODEL_ENTRY 받아오기 */
_bool CAnimator_Processor::Resolve_Model_Entry(ANIMATOR_DATA* pData, MESH_RENDERER_DATA*& pOutMeshRenderer, MODEL_ENTRY*& pOutModel)
{
    pOutMeshRenderer = nullptr;
    pOutModel = nullptr;

    if (!pData->hMeshRenderer.Is_Valid())
        return false;

    pOutMeshRenderer = To<MESH_RENDERER_DATA*>(m_pMeshRenderer_Processor->Get_DataPtr(COMPONENT_TYPE::MESH_RENDERER, pData->hMeshRenderer));
    IF_NULL_RETURN_MSG_BREAK(pOutMeshRenderer, false, "Can't find mesh renderer data");

    if (pOutMeshRenderer->hMesh == INVALID_ANIM_CLIP_INDEX)
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
    if (pModel->vecAnimClips.empty())
        return nullptr;

    if (pData->iAnimationClip == INVALID_ANIM_CLIP_INDEX)
        return nullptr;

    const uint32_t iClipIndex = pData->iAnimationClip;
    if (iClipIndex >= pModel->vecAnimClips.size())
        return nullptr;

    return &pModel->vecAnimClips[iClipIndex];
}

ANIMATION_CLIP_ENTRY* CAnimator_Processor::Resolve_Next_AnimationClip(const ANIMATOR_DATA* pData, MODEL_ENTRY* pModel)
{
    const uint32_t iClipIndex = pData->iNextAnimationClip;
    if (iClipIndex >= pModel->vecAnimClips.size())
        return nullptr;

    return &pModel->vecAnimClips[pData->iNextAnimationClip];
}

void CAnimator_Processor::Evaluate_AnimationChannels(ANIMATOR_DATA* pData, const MODEL_ENTRY& tModel, const ANIMATION_CLIP_ENTRY& tClip)
{
    IF_NULL_RETURN_MSG_BREAK(pData, , "pData is nullptr");

    /* 일단 모든 Bone을 기본 자세로 채운다. */
    /* 애니메이션 클립이 모든 Bone을 전부 채널로 갖고 있지 않을 수 있기 때문에 쓰레기값 방지 */
    const size_t iBoneCount = tModel.tSkeleton.bones.size();
    for (size_t i = 0; i < iBoneCount; ++i)
    {
        if (i < pData->boneLocalMatrices.size())
            pData->boneLocalMatrices[i] = tModel.tSkeleton.bones[i].matLocalBind;
    }

    /* 실제로 움직이는 본들에 대한 정보 : 애니메이션 채널이 있는 Bone만 계산하여 그 값을 덮어 쓴다. */
    /* 애니메이션 클립 안에 들어 있는 특정 본 하나의 애니메이션 정보 (e.g., 걷기 애니메이션의 오른쪽 정강이 애니메이션) */
    for (size_t i = 0; i < tClip.channels.size(); ++i)
    {
        /* 오른쪽 정강이 채널(=Bone)*/
        const ANIMATION_CHANNEL_ENTRY& tChannel = tClip.channels[i];                

        /* 해당 채널이 해당하는 BoneIndex의 로컬 행렬 */
        if (tChannel.iBoneIndex < 0 || To<size_t>(tChannel.iBoneIndex) >= pData->boneLocalMatrices.size())     
            continue;

        _float4x4 matLocal{};

        Evaluate_Channel_LocalTransform(pData, tChannel, (_uint)i, matLocal);

        if (pData->iNextAnimationClip != INVALID_ANIM_CLIP_INDEX
            && pData->iNextAnimationClip != INVALID_ANIM_CLIP_INDEX)
            Apply_Blend_To_LocalTransform(pData, tModel, (_uint)i, matLocal);

        pData->boneLocalMatrices[tChannel.iBoneIndex] = matLocal;
    }
}

/* 채널에 대한 키프레임 보간 및 전환 */
void CAnimator_Processor::Evaluate_Channel_LocalTransform(ANIMATOR_DATA* pData, const ANIMATION_CHANNEL_ENTRY& tChannel, _uint iChannelIndex, _float4x4& outLocalMatrix)
{
    outLocalMatrix = Math::Identity();

    /* 채널이 비었거나, 채널의 인덱스가 없는 경우 */
    if (tChannel.vecKeyFrames.empty() || iChannelIndex >= pData->currentKeyFrameIndices.size())
        return;

    /* 키 프레임이 하나인 경우 */
    if (tChannel.vecKeyFrames.size() == 1)
    {
        Math::Store(outLocalMatrix, Make_AffineMatrix(tChannel.vecKeyFrames[0]));
        return;
    }

    /* 해당 채널이 어느 키프레임까지 진행됐는지  */
    _uint& iCurrentKeyFrameIndex = pData->currentKeyFrameIndices[iChannelIndex];
    if (pData->fTrackPosition <= 0.f)
        iCurrentKeyFrameIndex = 0;

    const auto& vecKeyFrames = tChannel.vecKeyFrames;
    const ANIM_KEYFRAME& tLast = vecKeyFrames.back();

    /* 현재 애니메이션을 끝까지 재생한 상태 : 마지막 키프레임 유지 */
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

/* 애니메이션 전환 시 각 채널에 대한 블렌딩 */
void CAnimator_Processor::Apply_Blend_To_LocalTransform(ANIMATOR_DATA* pData, const MODEL_ENTRY& tModel, _uint iChannelIndex, _float4x4& inOutLocalMatrix)
{
    if (pData->fBlendElapsed > pData->fBlendDuration)
        return;

    if (pData->iNextAnimationClip == INVALID_ANIM_CLIP_INDEX)
        return;

    if (pData->iAnimationClip >= tModel.vecAnimClips.size() || pData->iNextAnimationClip >= tModel.vecAnimClips.size())
        return;

    const ANIMATION_CLIP_ENTRY& tCurClip = tModel.vecAnimClips[pData->iAnimationClip];
    if (iChannelIndex >= tCurClip.channels.size())
        return;

    const ANIMATION_CHANNEL_ENTRY& tCurChannel = tCurClip.channels[iChannelIndex];
    const _int iBoneIndex = tCurChannel.iBoneIndex;
    if (iBoneIndex < 0)
        return;

    /* 다음 애니메이션의 보간할 채널 찾기 */
    const ANIMATION_CLIP_ENTRY& tNextClip = tModel.vecAnimClips[pData->iNextAnimationClip];
    const ANIMATION_CHANNEL_ENTRY* pNextChannel = tNextClip.Find_Channel_ByBoneIndex(iBoneIndex);

    if (!pNextChannel)
        return;

    _float4x4 matNextLocal = Math::Identity();

    const auto& vecKeyFrames = pNextChannel->vecKeyFrames;
    if (vecKeyFrames.empty())
        return;

    /* 블렌딩할 키 프레임을 구하기 */
    const _float fNextTrackPosition = Get_NextClipTrackPosition(pData, tNextClip);

    if (vecKeyFrames.size() == 1) /* 키프레임이 하나면 유지 */
    {
        Math::Store(matNextLocal, Make_AffineMatrix(vecKeyFrames[0]));
    }
    else
    {
        if (fNextTrackPosition <= 0.f)
        {
            Math::Store(matNextLocal, Make_AffineMatrix(vecKeyFrames[0]));
        }
        else
        {
            const ANIM_KEYFRAME& tLast = vecKeyFrames.back();

            if (fNextTrackPosition >= tLast.fTrackPosition) /* 루프가 아니라면 마지막 키프레임 이후 해당 트랙 포지션 상태 유지 */
            {
                Math::Store(matNextLocal, Make_AffineMatrix(tLast));
            }
            else
            {
                size_t iNextKeyFrameIndex = 0;

                while ((iNextKeyFrameIndex + 1) < vecKeyFrames.size()
                    && fNextTrackPosition >= vecKeyFrames[iNextKeyFrameIndex + 1].fTrackPosition)
                {
                    ++iNextKeyFrameIndex;
                }

                if ((iNextKeyFrameIndex + 1) >= vecKeyFrames.size())
                {
                    Math::Store(matNextLocal, Make_AffineMatrix(vecKeyFrames.back()));
                }
                else /* 블렌딩할 다음 애니메이션의 키 프레임 사이 보간 */
                {
                    const ANIM_KEYFRAME& tLeft = vecKeyFrames[iNextKeyFrameIndex];
                    const ANIM_KEYFRAME& tRight = vecKeyFrames[iNextKeyFrameIndex + 1];

                    const _float fRatio = Compute_KeyFrameRatio(tLeft, tRight, fNextTrackPosition);
                    Math::Store(matNextLocal, Make_AffineMatrix(tLeft, tRight, fRatio));
                }
            }
        }
    }

    const _float fAlpha = Get_BlendAlpha(pData);
    Math::Store(inOutLocalMatrix, Blend_AffineMatrix(inOutLocalMatrix, matNextLocal, fAlpha));
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

        /* Why 2-step ? */
        /* matOffset을 곱하기 전까지는, Bone의 Transform 행렬이다. */
        /* 곱한 뒤로는 스키닝용 보정된 Bone의 Transform 행렬이다. */
        const _matrix matOffset = XMLoadFloat4x4(&tBone.matOffset);
        const _matrix matCombined = XMLoadFloat4x4(&pData->boneCombinedMatrices[i]);

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

_matrix CAnimator_Processor::Blend_AffineMatrix(const _float4x4& matCurrent, const _float4x4& matNext, _float fAlpha) const
{
    const _float fBlendAlpha = std::clamp(fAlpha, 0.f, 1.f);

    const _matrix matCurrentMatrix = XMLoadFloat4x4(&matCurrent);
    const _matrix matNextMatrix = XMLoadFloat4x4(&matNext);

    _vector vCurrentScale, vCurrentRotation, vCurrentTranslation;
    _vector vNextScale, vNextRotation, vNextTranslation;

    /* 행렬을 scale, rotation, translation으로 분해한다. */
    if (!XMMatrixDecompose(&vCurrentScale, &vCurrentRotation, &vCurrentTranslation, matCurrentMatrix))
        return matCurrentMatrix;

    if (!XMMatrixDecompose(&vNextScale, &vNextRotation, &vNextTranslation, matNextMatrix))
        return matCurrentMatrix;

    const _vector vBlendScale = XMVectorLerp(vCurrentScale, vNextScale, fBlendAlpha);
    const _vector vBlendRotation = XMQuaternionSlerp(vCurrentRotation, vNextRotation, fBlendAlpha);
    const _vector vBlendTranslation = XMVectorLerp(vCurrentTranslation, vNextTranslation, fBlendAlpha);

    return XMMatrixAffineTransformation(
        vBlendScale,
        XMVectorSet(0.f, 0.f, 0.f, 1.f),
        vBlendRotation,
        vBlendTranslation);
}


_float CAnimator_Processor::Get_AnimationTimePerSecond(const ANIMATION_CLIP_ENTRY& tClip) const
{
    return (tClip.fTickPerSecond <= 0.f) ? 1.f : tClip.fTickPerSecond;
}

/* 애니메이션 블렌드를 하면서 경과한 틱에 따른 트랙 포지션 */
_float CAnimator_Processor::Get_NextClipTrackPosition(const ANIMATOR_DATA* pData, const ANIMATION_CLIP_ENTRY& tNextClip) const
{
    _float fTrackPosition = pData->fBlendElapsed * Get_AnimationTimePerSecond(tNextClip) * pData->fPlaySpeed;

    if (tNextClip.fDuration <= 0.f)
        return 0.f;

    if (fTrackPosition >= tNextClip.fDuration)
        fTrackPosition = fmodf(fTrackPosition, tNextClip.fDuration);

    return fTrackPosition;
}

_float CAnimator_Processor::Get_BlendAlpha(const ANIMATOR_DATA* pData) const
{
    if (pData->iNextAnimationClip == INVALID_ANIM_CLIP_INDEX)
        return 1.f;

    if (pData->fBlendDuration <= 0.f)
        return 1.f;

    return std::clamp(pData->fBlendElapsed / pData->fBlendDuration, 0.f, 1.f);
}

_bool CAnimator_Processor::Is_LoopClip(const ANIMATOR_DATA* pData, uint32_t iClipIndex) const
{
    if (!pData)
        return false;

    auto it = pData->LoopMap.find(iClipIndex);
    if (it != pData->LoopMap.end())
        return it->second;

    /* NOTE : 지정되지 않았으면 루프 Off */
    return false;
}

uint64_t CAnimator_Processor::Make_AnimationClipBlendKey(uint32_t iFromClip, uint32_t iToClip) const
{
    return (To<uint64_t>(iFromClip) << 32) | static_cast<uint64_t>(iToClip);
}

void CAnimator_Processor::Try_Build_ClipNameMap(ANIMATOR_DATA* pData)
{
    if (!pData)
        return;
    if (!pData->hMeshRenderer.Is_Valid())
        return;

    MESH_RENDERER_DATA* pMrData = To<MESH_RENDERER_DATA*>(m_pMeshRenderer_Processor->Get_DataPtr(COMPONENT_TYPE::MESH_RENDERER, pData->hMeshRenderer));
    if (!pMrData)
        return;

    MODEL_ENTRY* pModel = SYS_RESOURCE.Get_Model(pMrData->hMesh);
    if (!pModel)
        return;

    pData->NameToClipIndex.clear();

    uint32_t iIndex = 0;
    for (const auto& clip : pModel->vecAnimClips)
        pData->NameToClipIndex.insert({ clip.strName, iIndex++ });
}

uint32_t CAnimator_Processor::Find_AnimationClip_By_Name(COMPONENT_HANDLE hComponent, const std::string& strClipName)
{
    auto pData = m_Pool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, INVALID_ANIM_CLIP_INDEX, "pData is nullptr");

    MESH_RENDERER_DATA* pMeshRenderer = nullptr;
    MODEL_ENTRY* pModel = nullptr;

    if (!Resolve_Model_Entry(pData, pMeshRenderer, pModel))
        return INVALID_ANIM_CLIP_INDEX;

    const auto& animClips = pModel->vecAnimClips;

    for (uint32_t i = 0; i < static_cast<uint32_t>(animClips.size()); ++i)
    {
        if (animClips[i].strName == strClipName)
            return i;
    }

    return INVALID_ANIM_CLIP_INDEX;
}


/* 설정한 블렌딩 시간 넣기 */
_float CAnimator_Processor::Get_BlendDuration(const ANIMATOR_DATA* pData, uint32_t iFromClip, uint32_t iToClip) const
{
    if (!pData)
        return 0.f;

    MESH_RENDERER_DATA* pMrData = To<MESH_RENDERER_DATA*>(m_pMeshRenderer_Processor->Get_DataPtr(COMPONENT_TYPE::MESH_RENDERER, pData->hMeshRenderer));
    if (!pMrData)
        return 0.f;

    MODEL_ENTRY* pModel = SYS_RESOURCE.Get_Model(pMrData->hMesh);
    if (!pModel)
        return 0.f;

    if (iToClip >= pModel->vecAnimClips.size())
        return 0.f;

    _float fBlendDuration = m_fEnsureBlendingTime;

    const uint64_t iBlendKey = Make_AnimationClipBlendKey(iFromClip, iToClip);
    auto it = pData->BlendMap.find(iBlendKey);
    if (it != pData->BlendMap.end())
        fBlendDuration = it->second;

    if (fBlendDuration < 0.f)
        fBlendDuration = 0.f;

    const auto& toClip = pModel->vecAnimClips[iToClip];
    if (toClip.fDuration > 0.f)
        fBlendDuration = std::fminf(fBlendDuration, toClip.fDuration);
    else
        fBlendDuration = 0.f;

    return fBlendDuration;
}

void CAnimator_Processor::Set_Loop(ANIMATOR_DATA* pData, uint32_t iCurClip, _bool bLoop)
{
    if (!pData)
        return ;

    pData->LoopMap[iCurClip] = bLoop;
}

std::unique_ptr<CAnimator_Processor> CAnimator_Processor::Create()
{
    auto pInstance = std::make_unique<CAnimator_Processor>();

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}

