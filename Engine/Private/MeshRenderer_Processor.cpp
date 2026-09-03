#include "MeshRenderer_Processor.h"
#include "Engine_Log.h"
#include "Resource_System.h"
#include "Component_Spec.h"
#include "Transform_Processor.h"
#include "Animator_Processor.h"
#include "Engine_Math.h"
#include "GameObject.h"
#include "Texture.h"
#include "Animator.h"
#include "RandomUtil.h"
#include "CRender_System.h"
#include "Render_Context.h"

CMeshRenderer_Processor::CMeshRenderer_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice(pDevice), m_pContext(pContext)
{
}

CMeshRenderer_Processor::~CMeshRenderer_Processor()
{
}

HRESULT CMeshRenderer_Processor::Initialize()
{
    /* 팩토리 등록 */
    {
        SYS_COMPONENT.Register_InitialSpecFactory<CMeshRenderer, MESH_RENDERER_SPEC>();
        SYS_COMPONENT.Register_BuildSpecFacotry<CMeshRenderer>();
    }

    /* 프로페서에서 new 생성하는 객체는 직접 해제해준다. */
    m_Pool.Subscribe_OnDeallocate(&CMeshRenderer_Processor::Reset_Data_On_Deallocate, this);

    m_hNonAnimOutlineMat = SYS_RESOURCE.Load_Material(DEFAULT_ASSET_GUID::MATERIAL_OUTLINE);
    m_hAnimOutlineMat = SYS_RESOURCE.Load_Material(DEFAULT_ASSET_GUID::MATERIAL_OUTLINE_ANIM);

    m_iCurFrame = 0;

    return S_OK;
}

HRESULT CMeshRenderer_Processor::Late_Initialize()
{
    m_pTransformProcessor = SYS_COMPONENT.Bind_Processor<CTransform_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pTransformProcessor, E_FAIL, "pTransformProcessor is nullptr");

    m_pAnimatorProcessor = SYS_COMPONENT.Bind_Processor<CAnimator_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pAnimatorProcessor, E_FAIL, "mpAnimatorProcessor is nullptr");

    return S_OK;
}

void CMeshRenderer_Processor::Update(_float fDT)
{
    const auto& Pages = m_Pool.GetPages();

    for (const auto& upPage : Pages)
    {
        auto* pPage = upPage.get();
        if (!pPage) continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;
            auto* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable)
                continue;
            if (pData->eMode != MESH_MODE::PARTICLE)
                continue;

            Update_Particle(pData, fDT);
        }
    }
}

void CMeshRenderer_Processor::LateUpdate(_float fDT)
{
}

void CMeshRenderer_Processor::Build_RenderQueue(vector<DRAW_CMD>& outCmds)
{
    if (!m_pTransformProcessor)
        return;

    //_uint iTotalRendererCount = 0;
    //_uint iCulledRendererCount = 0;
    //_uint iDrawRendererCount = 0;
    //_uint iParticleCount = 0;

    const _bool bCanFrustumCull = Update_Frustum();

    const auto& Pages = m_Pool.GetPages();
    for (const auto& upPage : Pages)
    {
        auto* pPage = upPage.get();
        if (!pPage)
            continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            auto* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable)
                continue;

            //iTotalRendererCount++;

            /* 파티클은 컬링하지 않음 */
            if (pData->eMode == MESH_MODE::PARTICLE)
            {
                if (!pData->bParticlePlaying || pData->bParticleFinished)
                    continue;

                if (!Ensure_ParticleRuntime(pData))
                    continue;

                //++iParticleCount;

                DRAW_CMD cmd{};
                cmd.mesh.eMode = MESH_MODE::PARTICLE;
                cmd.mesh.hTransform = pData->hTransform;
                cmd.mesh.iParticleRuntime = pData->iParticleRuntime;
                cmd.eLayer = pData->layer;
                cmd.mesh.flags = pData->flags;

                outCmds.push_back(std::move(cmd));
                continue;
            }

            if (pData->hMesh == INVALID_HANDLE_UINT)
                continue;

            COMPONENT_HANDLE hReferenceAnimator = INVALID_HANDLE;

            if (pData->hAnimator.Is_Valid())
                hReferenceAnimator = pData->hAnimator;
            else if (pData->hSkinningSourceAnimator.Is_Valid())
                hReferenceAnimator = pData->hSkinningSourceAnimator;
            else if (pData->hAttachSourceAnimator.Is_Valid())
                hReferenceAnimator = pData->hAttachSourceAnimator;

            DRAW_CMD tCmd = DRAW_CMD::Create_Mesh(
                pData->hMesh,
                pData->hMaterial,
                pData->hTransform,
                hReferenceAnimator,
                pData->flags,
                0,
                0);

            tCmd.mesh.hPerObjectParams = pData->hPerObjectParams;
            tCmd.sortKey = Make_SortKey(*pData);
            tCmd.mesh.eMode = pData->eMode;
            tCmd.eLayer = pData->layer;
            tCmd.mesh.pMeshRendererData = pData;

            if (pData->bUseDissolvePass)
                tCmd.mesh.iForcedPassIndex = 3; // DefaultPass=0, Outline/Dummy=1, ShadowPass=2, DissolvePass=3
            if (pData->hSkinningSourceAnimator.Is_Valid())
            {
                if (Build_SkinnedPart_BoneMatrices(pData))
                    tCmd.mesh.pSkinningMatrices = &pData->vecSkinningBoneMatrices;
            }
            else if (pData->hAttachSourceAnimator.Is_Valid())
            {
                if (Build_Attach_BoneMatrix(pData))
                    tCmd.mesh.matAttach = pData->matFinalAttach;
            }

            /*
            * Shadow pass는 카메라 프러스텀 컬링보다 먼저 넣는다
            * 카메라에는 안 보이는 오브젝트라도 라이트 기준에서는 그림자를 만들어야 할 수 있음.
            */
            if (pData->eShadowType != SHADOW_TYPE::NONE)
            {
                DRAW_CMD tShadowCmd = tCmd;

                tShadowCmd.eLayer =
                    (pData->eShadowType == SHADOW_TYPE::STATIC)
                    ? RENDER_LAYER::SHADOW_STATIC
                    : RENDER_LAYER::SHADOW_DYNAMIC;

                tShadowCmd.mesh.eShadowType = pData->eShadowType;
                tShadowCmd.mesh.iForcedPassIndex = 2; // DefaultPass=0, OutlinePass or dummy=1, ShadowPass=2

                outCmds.push_back(tShadowCmd);
            }

            /*
            * 여기부터는 일반 카메라 렌더링용 컬링
            * 여기서 continue 되어도 위에서 shadow cmd는 이미 들어갔음.
            */
            if (bCanFrustumCull)
            {
                if (Is_Culled_Renderer_By_Frustum(pData))
                {
                    //++iCulledRendererCount;
                    continue;
                }
            }

            //++iDrawRendererCount;

            outCmds.push_back(tCmd);

            if ((pData->extraPassFlags & To<uint32_t>(EXTRA_RENDER_PASS::OUTLINE)) != 0)
            {
                const _bool bAnimMesh =
                    pData->hAnimator.Is_Valid() ||
                    pData->hSkinningSourceAnimator.Is_Valid();

                uint32_t hOutlineMat = INVALID_HANDLE_UINT;

                if (bAnimMesh)
                {
                    if (m_hAnimOutlineMat == INVALID_HANDLE_UINT)
                        continue;

                    hOutlineMat = m_hAnimOutlineMat;
                }
                else
                {
                    if (m_hNonAnimOutlineMat == INVALID_HANDLE_UINT)
                        continue;

                    hOutlineMat = m_hNonAnimOutlineMat;
                }

                DRAW_CMD tOutlineCmd = tCmd;
                tOutlineCmd.mesh.hMaterial = hOutlineMat;
                tOutlineCmd.mesh.hPerObjectParams = pData->hPerObjectParams;
                tOutlineCmd.sortKey += 1;

                outCmds.push_back(tOutlineCmd);
            }
        }
    }

    m_iCurFrame++;

    //LOG_INFO(
    //    "FrustumCull Total=%u, Draw=%u, Culled=%u, Particle=%u, Cmds=%u",
    //    iTotalRendererCount,
    //    iDrawRendererCount,
    //    iCulledRendererCount,
    //    iParticleCount,
    //    static_cast<_uint>(outCmds.size())
    //);
}

HRESULT CMeshRenderer_Processor::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec)
{
    IF_TRUE_RETURN_MSG_BREAK(eComType != COMPONENT_TYPE::MESH_RENDERER, E_FAIL, "Wrong component type.");
    MESH_RENDERER_DATA* pData = m_Pool.Get_Data_By_Handle(handle);
    IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "Invalid MeshRenderer handle in Initialize_From_Spec");

    const auto* pMeshSpec = SCAST(const MESH_RENDERER_SPEC*, pSpec);
    IF_NULL_RETURN_MSG_BREAK(pMeshSpec, E_FAIL, "Invalid MeshRenderer spec");

    pData->hMesh = INVALID_HANDLE_UINT;
    pData->hMaterial = INVALID_HANDLE_UINT;
    pData->hParticle = INVALID_HANDLE_UINT;
    pData->iParticleRuntime = INVALID_HANDLE_UINT;

    if (pMeshSpec->meshGUID.Is_Valid())
        pData->hMesh = SYS_RESOURCE.Load_Mesh(pMeshSpec->meshGUID);

    if (pMeshSpec->materialGUID.Is_Valid())
        pData->hMaterial = SYS_RESOURCE.Load_Material(pMeshSpec->materialGUID);

    if (pMeshSpec->particleGUID.Is_Valid())
        pData->hParticle = SYS_RESOURCE.Load_Particle(pMeshSpec->particleGUID);

    pData->vecOverrideMaterials = pMeshSpec->vecOverrideMaterials;

    if (SYS_RESOURCE.Is_ModelHandle(pData->hMesh))
    {
        const MODEL_ENTRY* pModel = SYS_RESOURCE.Get_Model(pData->hMesh);
        if (pModel)
            pData->vecOverrideMaterials.resize(pModel->parts.size(), INVALID_HANDLE_UINT);
    }

    pData->flags = pMeshSpec->flags;
    pData->layer = pMeshSpec->layer;
    pData->sortZ = pMeshSpec->sortZ;
    pData->bEnable = pMeshSpec->bEnable;

    pData->eMode = pMeshSpec->eMode;
    pData->strAttachBoneName = pMeshSpec->strAttachBoneName;

    pData->bParticlePlaying = pMeshSpec->bParticlePlaying;
    pData->vParticlePivot = pMeshSpec->vParticlePivot;

    pData->extraPassFlags = pMeshSpec->extraPassFlags;
    pData->eShadowType = pMeshSpec->eShadowType;

    HRESULT hr = S_OK;

    if (pData->eMode == MESH_MODE::PARTS)
        hr = Resolve_SkinningReference(handle) ? S_OK : E_FAIL;
    else if (pData->eMode == MESH_MODE::ATTACH)
        hr = Resolve_AttachReference(handle) ? S_OK : E_FAIL;

    IF_FAIL_RETURN_MSG_BREAK(hr, hr, "mesh renderer failed to initialize spec");

    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CMeshRenderer_Processor::Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    IF_TRUE_RETURN_MSG_BREAK(eComType != COMPONENT_TYPE::MESH_RENDERER, nullptr, "Wrong component type.");

    MESH_RENDERER_DATA* pData = m_Pool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, nullptr, "Invalid MeshRenderer handle in Build_Spec");

    auto spec = std::make_unique<MESH_RENDERER_SPEC>();

    if (pData->hMesh != INVALID_HANDLE_UINT)
    {
        if (SYS_RESOURCE.Is_ModelHandle(pData->hMesh))
        {
            MODEL_ENTRY* pModel = SYS_RESOURCE.Get_Model(pData->hMesh);
            if (pModel)
                spec->meshGUID = pModel->tGUID;
        }
        else
        {
            MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(pData->hMesh);
            if (pMesh)
                spec->meshGUID = pMesh->tGUID;
        }
    }

    if (pData->hMaterial != INVALID_HANDLE_UINT)
    {
        MATERIAL_ENTRY* pMat = SYS_RESOURCE.Get_Material(pData->hMaterial);
        if (pMat)
            spec->materialGUID = pMat->tGUID;
    }

    if (pData->hParticle != INVALID_HANDLE_UINT)
    {
        PARTICLE_ENTRY* pParticle = SYS_RESOURCE.Get_Particle(pData->hParticle);
        if (pParticle)
            spec->particleGUID = pParticle->tGUID;
    }

    spec->bParticlePlaying = pData->bParticlePlaying;
    spec->vParticlePivot = pData->vParticlePivot;

    spec->flags = pData->flags;
    spec->layer = pData->layer;
    spec->sortZ = pData->sortZ;
    spec->bEnable = pData->bEnable;

    spec->eMode = pData->eMode;
    spec->strAttachBoneName = pData->strAttachBoneName;

    spec->vecOverrideMaterials = pData->vecOverrideMaterials;
    spec->extraPassFlags = pData->extraPassFlags;
    spec->eShadowType = pData->eShadowType;

    return spec;
}

void CMeshRenderer_Processor::Initialize_Component_Data(COMPONENT_HANDLE hComponent)
{
    auto pData = m_Pool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, , "pData is nullptr");

    CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
    IF_NULL_RETURN_MSG_BREAK(pObj, , "pObj is nullptr");

    if (pData->hPerObjectParams == INVALID_HANDLE_UINT)
        pData->hPerObjectParams = SYS_RESOURCE.Alloc_PerObjectParamBlock();

    pData->hTransform = pObj->Get_Component<CTransform>().Get_Handle();

    pData->vecOverrideMaterials.clear();

    Clear_SkinningReference(pData);
    Clear_AttachReference(pData);
}

uint64_t CMeshRenderer_Processor::Make_SortKey(const MESH_RENDERER_DATA& tData) const
{
    /* [63:60] layer (4 bit)*/
    /* [59:28] material(32 bit) */
    /* [27:0 ] mesh(28 bit) */
    uint64_t key = 0;
    key |= (uint64_t)((uint8_t)tData.layer & 0xF) << 60;
    key |= (uint64_t)(tData.hMaterial) << 28;
    key |= (uint64_t)(tData.hMesh & 0x0FFFFFFF);
    return key;
}

_bool CMeshRenderer_Processor::Resolve_References(COMPONENT_HANDLE hComponent)
{
    auto pData = m_Pool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, false, "pData is nullptr");

    Clear_SkinningReference(pData);
    Clear_AttachReference(pData);

    if (pData->eMode == MESH_MODE::PARTS)
        return Resolve_SkinningReference(hComponent);
    if (pData->eMode == MESH_MODE::ATTACH)
        return Resolve_AttachReference(hComponent);

    return true;
}

void CMeshRenderer_Processor::Reset_Data_On_Deallocate(COMPONENT_HANDLE hScript, MESH_RENDERER_DATA* pData)
{
    SYS_RESOURCE.Free_PerObjectParamBlock(pData->hPerObjectParams);
}

/* 파츠 skeleton의 i번 bone이 부모 skeleton의 몇 번 bone을 봐야 하는지 */
_bool CMeshRenderer_Processor::Build_Skinning_BoneRemap(MESH_RENDERER_DATA* pData)
{
    IF_NULL_RETURN_MSG_BREAK(pData, false, "pData is nullptr");

    if (!pData->hSkinningSourceAnimator.Is_Valid())
        return false;

    if (!SYS_RESOURCE.Is_ModelHandle(pData->hMesh))     /* 뼈를 가져오기 위해 반드시 모델이어야 한다. */
        return false;

    /* 참조 중인 애니메이터 */
    ANIMATOR_DATA* pParentAnim =
        To<ANIMATOR_DATA*>(m_pAnimatorProcessor->Get_DataPtr(COMPONENT_TYPE::ANIMATOR, pData->hSkinningSourceAnimator));

    IF_NULL_RETURN_MSG_BREAK(pParentAnim, false, "pParentAnim is nullptr");

    MESH_RENDERER_DATA* pParentMeshRenderer = nullptr;
    MODEL_ENTRY* pParentModel = nullptr;
    MODEL_ENTRY* pPartModel = nullptr;

    {
        pParentMeshRenderer = To<MESH_RENDERER_DATA*>(Get_DataPtr(COMPONENT_TYPE::MESH_RENDERER, pParentAnim->hMeshRenderer));
        IF_NULL_RETURN_MSG_BREAK(pParentMeshRenderer, false, "pParentMeshRenderer is nullptr");

        if (!SYS_RESOURCE.Is_ModelHandle(pParentMeshRenderer->hMesh))
            return false;

        pParentModel = SYS_RESOURCE.Get_Model(pParentMeshRenderer->hMesh);
        IF_NULL_RETURN_MSG_BREAK(pParentModel, false, "pParentModel is nullptr");
    }
    {
        pPartModel = SYS_RESOURCE.Get_Model(pData->hMesh);
        IF_NULL_RETURN_MSG_BREAK(pPartModel, false, "pPartModel is nullptr");
    }

    const auto& partSkeleton = pPartModel->tSkeleton;
    const auto& parentSkeleton = pParentModel->tSkeleton;

    if ((partSkeleton.iRootBoneIndex == -1)/* invalid */ || (parentSkeleton.iRootBoneIndex == -1)/* invalid */)
        return false;

    pData->vecSkinningBoneRemap.clear();
    pData->vecSkinningBoneRemap.resize(partSkeleton.bones.size(), INVALID_HANDLE_UINT);

    /* 부모의 스켈레톤에서 내(=파츠)가 참조 중인 뼈의 인덱스 매핑해오기 */
    for (uint32_t i = 0; i < To<uint32_t>(partSkeleton.bones.size()); ++i)
    {
        const std::string& strBoneName = partSkeleton.bones[i].strName;
        auto itFound = parentSkeleton.BoneNameToIndex.find(strBoneName);
        if (itFound == parentSkeleton.BoneNameToIndex.end())
        {
            pData->vecSkinningBoneRemap[i] = INVALID_HANDLE_UINT;
            continue;
        }

        pData->vecSkinningBoneRemap[i] = itFound->second;
    }

    pData->vecSkinningBoneMatrices.resize(partSkeleton.bones.size());
    return true;
}

/* NOTE : Parts는 정점이 Bone 영향을 받아 변형되는 메쉬이므로, matOffset이 곱해진 finalBoneMatrices를 사용해야 한다. */
_bool CMeshRenderer_Processor::Build_SkinnedPart_BoneMatrices(MESH_RENDERER_DATA* pData)
{
    IF_NULL_RETURN_MSG_BREAK(pData, false, "pData is nullptr");

    if (!pData->hSkinningSourceAnimator.Is_Valid())
        return false;

    ANIMATOR_DATA* pParentAnim =
        To<ANIMATOR_DATA*>(m_pAnimatorProcessor->Get_DataPtr(COMPONENT_TYPE::ANIMATOR, pData->hSkinningSourceAnimator));
    IF_NULL_RETURN_MSG_BREAK(pParentAnim, false, "pParentAnim is nullptr");

    if (pParentAnim->finalBoneMatrices.empty())
        return false;

    if (pData->vecSkinningBoneRemap.empty())
    {
        if (!Build_Skinning_BoneRemap(pData)) /* 비었다면 재빌드 시도 */
            return false;
    }

    /* 사이즈 통일 */
    if (pData->vecSkinningBoneMatrices.size() != pData->vecSkinningBoneRemap.size())
        pData->vecSkinningBoneMatrices.resize(pData->vecSkinningBoneRemap.size());

    for (uint32_t i = 0; i < To<uint32_t>(pData->vecSkinningBoneRemap.size()); ++i)
    {
        const uint32_t iParentBoneIndex = pData->vecSkinningBoneRemap[i];

        if (iParentBoneIndex == INVALID_HANDLE_UINT || iParentBoneIndex >= pParentAnim->finalBoneMatrices.size())
        {
            pData->vecSkinningBoneMatrices[i] = Math::Identity();
            continue;
        }

        /* 파츠의 스켈레톤 인덱스 -> 부모의 스켈레톤 인덱스로 변환하여 읽기 */
        pData->vecSkinningBoneMatrices[i] = pParentAnim->finalBoneMatrices[iParentBoneIndex];
    }

    return true;
}

_bool CMeshRenderer_Processor::Find_Attach_BoneIndex(MESH_RENDERER_DATA* pData)
{
    IF_NULL_RETURN_MSG_BREAK(pData, false, "pData is nullptr");

    if (!pData->hAttachSourceAnimator.Is_Valid())
        return false;

    if (pData->strAttachBoneName.empty())
        return false;

    /* 참조 중인 애니메이터 */
    ANIMATOR_DATA* pParentAnim =
        To<ANIMATOR_DATA*>(m_pAnimatorProcessor->Get_DataPtr(COMPONENT_TYPE::ANIMATOR, pData->hAttachSourceAnimator));
    IF_NULL_RETURN_MSG_BREAK(pParentAnim, false, "pParentAnim is nullptr");

    MESH_RENDERER_DATA* pParentMeshRenderer = nullptr;
    pParentMeshRenderer = To<MESH_RENDERER_DATA*>(Get_DataPtr(COMPONENT_TYPE::MESH_RENDERER, pParentAnim->hMeshRenderer));
    IF_NULL_RETURN_MSG_BREAK(pParentMeshRenderer, false, "pParentMeshRenderer is nullptr");

    if (!SYS_RESOURCE.Is_ModelHandle(pParentMeshRenderer->hMesh))           /* 규약 : 뼈가 존재하는 '모델'에 붙인다 */
        return false;

    MODEL_ENTRY* pParentModel = SYS_RESOURCE.Get_Model(pParentMeshRenderer->hMesh);
    IF_NULL_RETURN_MSG_BREAK(pParentModel, false, "pParentModel is nullptr");

    const auto& parentSkeleton = pParentModel->tSkeleton;
    if (parentSkeleton.iRootBoneIndex == -1)
        return false;

    auto itFound = parentSkeleton.BoneNameToIndex.find(pData->strAttachBoneName);
    if (itFound == parentSkeleton.BoneNameToIndex.end())
        return false;

    pData->iAttachBoneIdx = itFound->second;
    return true;
}

/* NOTE : Attach는 별도의 메쉬를 가진 오브젝트가 Bone의 Transform에 붙어야 하므로, boneCombinedMatrices를 이용해야 한다. */
_bool CMeshRenderer_Processor::Build_Attach_BoneMatrix(MESH_RENDERER_DATA* pData)
{
    IF_NULL_RETURN_MSG_BREAK(pData, false, "pData is nullptr");

    if (!pData->hAttachSourceAnimator.Is_Valid())
        return false;

    ANIMATOR_DATA* pParentAnim =
        To<ANIMATOR_DATA*>(m_pAnimatorProcessor->Get_DataPtr(COMPONENT_TYPE::ANIMATOR, pData->hAttachSourceAnimator));
    IF_NULL_RETURN_MSG_BREAK(pParentAnim, false, "pParentAnim is nullptr");

    if (pParentAnim->boneCombinedMatrices.empty())
        return false;

    if (pData->iAttachBoneIdx == INVALID_HANDLE_UINT)
    {
        if (!Find_Attach_BoneIndex(pData))
            return false;
    }

    const uint32_t iAttachBoneIndex = pData->iAttachBoneIdx;

    if (iAttachBoneIndex >= pParentAnim->boneCombinedMatrices.size())
    {
        pData->matFinalAttach = Math::Identity();
        pData->iAttachBoneIdx = INVALID_HANDLE_UINT;
        return false;
    }

    MESH_RENDERER_DATA* pParentMeshRenderer =
        To<MESH_RENDERER_DATA*>(Get_DataPtr(COMPONENT_TYPE::MESH_RENDERER, pParentAnim->hMeshRenderer));
    IF_NULL_RETURN_MSG_BREAK(pParentMeshRenderer, false, "pParentMeshRenderer is nullptr");

    const auto* pParentTrData =
        To<TRANSFORM_DATA*>(m_pTransformProcessor->Get_DataPtr(COMPONENT_TYPE::TRANSFORM, pParentMeshRenderer->hTransform));
    IF_NULL_RETURN_MSG_BREAK(pParentTrData, false, "pParentTrData is nullptr");

    const auto* pChildTrData =
        To<TRANSFORM_DATA*>(m_pTransformProcessor->Get_DataPtr(COMPONENT_TYPE::TRANSFORM, pData->hTransform));
    IF_NULL_RETURN_MSG_BREAK(pChildTrData, false, "pChildTrData is nullptr");

    Math::Store(
        pData->matFinalAttach,
        Math::Load(pChildTrData->matWorld) *
        Math::Load(pParentAnim->boneCombinedMatrices[iAttachBoneIndex]) *
        Math::Load(pParentTrData->matWorld)
    );

    return true;
}

void CMeshRenderer_Processor::Clear_SkinningReference(MESH_RENDERER_DATA* pData)
{
    IF_NULL_RETURN_MSG_BREAK(pData, , "pData is nullptr");

    pData->hSkinningSourceAnimator = INVALID_HANDLE;
    pData->vecSkinningBoneRemap.clear();
    pData->vecSkinningBoneMatrices.clear();

    pData->eMode = MESH_MODE::NONE;
}

void CMeshRenderer_Processor::Clear_AttachReference(MESH_RENDERER_DATA* pData)
{
    IF_NULL_RETURN_MSG_BREAK(pData, , "pData is nullptr");

    pData->hAttachSourceAnimator = INVALID_HANDLE;
    pData->iAttachBoneIdx = INVALID_HANDLE_UINT;
    pData->matFinalAttach = Math::Identity();

    pData->eMode = MESH_MODE::NONE;
}

/* 에디터에서 편집을 통해 설정 */
_bool CMeshRenderer_Processor::Resolve_SkinningReference(COMPONENT_HANDLE hComponent)
{
    auto pData = m_Pool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, false, "pData is nullptr");

    Clear_AttachReference(pData);
    Clear_SkinningReference(pData);

    CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
    IF_NULL_RETURN_MSG_BREAK(pObj, false, "pObj is nullptr");

    pData->hTransform = pObj->Get_Component<CTransform>().Get_Handle();

    /* 자기 자신이 Animator를 갖는 일반 animated mesh면 skinning source는 필요 없음 */
    if (pData->hAnimator.Is_Valid())
        return true;

    CGameObject* pParent = pObj->Get_Parent();
    if (pParent == nullptr)
        return false;

    /* 렌더 시 부모의 월드 행렬과 부모의 애니메이터를 이용해야 하므로 부모의 트랜스폼을 저장해야 한다. */
    CTransform parentTransform = pParent->Get_Component<CTransform>();
    if (!parentTransform.Is_Valid())
        return false;
    CAnimator parentAnim = pParent->Get_Component<CAnimator>();
    if (!parentAnim.Is_Valid())
        return false;

    pData->hTransform = parentTransform.Get_Handle();
    pData->hSkinningSourceAnimator = parentAnim.Get_Handle();

    if (!Build_Skinning_BoneRemap(pData))
    {
        Clear_SkinningReference(pData);
        return false;
    }

    pData->eMode = MESH_MODE::PARTS;

    return true;
}

_bool CMeshRenderer_Processor::Resolve_AttachReference(COMPONENT_HANDLE hComponent)
{
    auto pData = m_Pool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, false, "pData is nullptr");

    Clear_AttachReference(pData);
    Clear_SkinningReference(pData);

    if (pData->strAttachBoneName.empty())
        return false;

    CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
    IF_NULL_RETURN_MSG_BREAK(pObj, false, "pObj is nullptr");

    /* Build_Attach_BoneMatrix()에서 나의 월드 행렬에 부모의 월드 행렬을 곱해주므로, attach는 나의 트랜스폼을 참조하는 게 맞다. */
    pData->hTransform = pObj->Get_Component<CTransform>().Get_Handle();

    CGameObject* pParent = pObj->Get_Parent();
    if (pParent == nullptr)
        return false;

    CAnimator parentAnim = pParent->Get_Component<CAnimator>();
    if (!parentAnim.Is_Valid())
        return false;

    pData->hAttachSourceAnimator = parentAnim.Get_Handle();

    if (!Find_Attach_BoneIndex(pData))
    {
        Clear_AttachReference(pData);
        return false;
    }

    pData->eMode = MESH_MODE::ATTACH;

    return true;
}

void CMeshRenderer_Processor::Update_Particle(MESH_RENDERER_DATA* pData, _float fDT)
{
    if (!pData)
        return;

    if (pData->eMode != MESH_MODE::PARTICLE)
        return;

    if (!Ensure_ParticleRuntime(pData))
        return;

    if (pData->bParticleResetRequested)
    {
        Reset_ParticleRuntime(pData);
        pData->bParticleResetRequested = false;
    }

    if (pData->bParticlePlaying == false)
        return;

    PARTICLE_RUNTIME* pRuntime = Get_ParticleRuntime(pData->iParticleRuntime);
    PARTICLE_ENTRY* pParticle = SYS_RESOURCE.Get_Particle(pData->hParticle);

    if (!pRuntime || !pParticle)
        return;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (FAILED(m_pContext->Map(pRuntime->pInstanceVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        return;

    auto* pInstance = To<VTXPARTICLE_INSTANCE*>(mapped.pData);

    _bool bAnyAlive = false;

    for (_uint i = 0; i < pRuntime->iNumInstances; ++i)
    {
        pRuntime->vecInstances[i].vLifeTime.y += fDT;

        if (pRuntime->vecInstances[i].vLifeTime.y < pRuntime->vecInstances[i].vLifeTime.x)
            bAnyAlive = true;

        if (pParticle->eSimulation == PARTICLE_SIMULATION::DROP)
        {
            pRuntime->vecInstances[i].vTranslation.y -= pRuntime->vecSpeeds[i] * fDT;
        }
        else if (pParticle->eSimulation == PARTICLE_SIMULATION::SPREAD)
        {
            _vector vPos = XMLoadFloat4(&pRuntime->vecInstances[i].vTranslation);
            _vector vPivot = XMLoadFloat3(&pData->vParticlePivot);
            _vector vDir = XMVectorSetW(vPos - vPivot, 0.f);

            if (!XMVector3NearEqual(vDir, XMVectorZero(), XMVectorReplicate(0.0001f)))
            {
                vPos += XMVector3Normalize(vDir) * pRuntime->vecSpeeds[i] * fDT;
                XMStoreFloat4(&pRuntime->vecInstances[i].vTranslation, vPos);
            }
        }
        else if (pParticle->eSimulation == PARTICLE_SIMULATION::SLIDE)
        {
            _vector vPos = XMLoadFloat4(&pRuntime->vecInstances[i].vTranslation);
            _vector vDir = XMLoadFloat4(&pRuntime->vecDirections[i]);

            vPos += vDir * pRuntime->vecSpeeds[i] * fDT;
            XMStoreFloat4(&pRuntime->vecInstances[i].vTranslation, vPos);
        }
        else if (pParticle->eSimulation == PARTICLE_SIMULATION::DRIFT)
        {
            _vector vPos = XMLoadFloat4(&pRuntime->vecInstances[i].vTranslation);
            _vector vDir = XMLoadFloat4(&pRuntime->vecDirections[i]);

            vPos += vDir * pRuntime->vecSpeeds[i] * fDT;
            XMStoreFloat4(&pRuntime->vecInstances[i].vTranslation, vPos);
        }
        else if (pParticle->eSimulation == PARTICLE_SIMULATION::DUST)
        {
            _vector vPos = XMLoadFloat4(&pRuntime->vecInstances[i].vTranslation);
            _vector vPivot = XMLoadFloat3(&pData->vParticlePivot);
            _vector vDiff = vPos - vPivot;
            if (XMVectorGetY(vDiff) < 0.f)
                vDiff = XMVectorSetY(vDiff, 0.f);

            _vector vDir = XMVectorSetW(vDiff, 0.f);

            if (!XMVector3NearEqual(vDir, XMVectorZero(), XMVectorReplicate(0.0001f)))
            {
                vPos += XMVector3Normalize(vDir) * pRuntime->vecSpeeds[i] * fDT;
                XMStoreFloat4(&pRuntime->vecInstances[i].vTranslation, vPos);
            }
        }

        /* life 타임 끝 */
        if (pRuntime->vecInstances[i].vLifeTime.y >= pRuntime->vecInstances[i].vLifeTime.x)
        {
            if (pParticle->isLoop)
            {
                const _float fScaleX = CRandomUtil::Get_Float(pParticle->vScaleX.x, pParticle->vScaleX.y);
                const _float fScaleY = CRandomUtil::Get_Float(pParticle->vScaleY.x, pParticle->vScaleY.y);

                pRuntime->vecSpeeds[i] = CRandomUtil::Get_Float(pParticle->vSpeed.x, pParticle->vSpeed.y);

                pRuntime->vecInstances[i].vRight = _float4(fScaleX, 0.f, 0.f, 0.f);
                pRuntime->vecInstances[i].vUp = _float4(0.f, fScaleY, 0.f, 0.f);
                pRuntime->vecInstances[i].vLook = _float4(0.f, 0.f, 1.f, 0.f);

                pRuntime->vecInstances[i].vTranslation = _float4(
                    CRandomUtil::Get_Float(pParticle->vCenter.x - pParticle->vRange.x * 0.5f, pParticle->vCenter.x + pParticle->vRange.x * 0.5f),
                    CRandomUtil::Get_Float(pParticle->vCenter.y - pParticle->vRange.y * 0.5f, pParticle->vCenter.y + pParticle->vRange.y * 0.5f),
                    CRandomUtil::Get_Float(pParticle->vCenter.z - pParticle->vRange.z * 0.5f, pParticle->vCenter.z + pParticle->vRange.z * 0.5f),
                    1.f);

                pRuntime->vecInstances[i].vLifeTime = _float2(
                    CRandomUtil::Get_Float(pParticle->vLifeTime.x, pParticle->vLifeTime.y),
                    0.f);

                if (pParticle->eSimulation == PARTICLE_SIMULATION::SLIDE)
                {
                    _vector vForward = XMLoadFloat3(&pData->vParticleForward);
                    if (XMVector3NearEqual(vForward, XMVectorZero(), XMVectorReplicate(0.0001f)))
                        vForward = XMVectorSet(0.f, 0.f, 1.f, 0.f);

                    vForward = XMVector3Normalize(vForward);

                    _vector vBack = -vForward;
                    _vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
                    _vector vRightBase = XMVector3Cross(vWorldUp, vBack);

                    if (XMVector3NearEqual(vRightBase, XMVectorZero(), XMVectorReplicate(0.0001f)))
                        vRightBase = XMVectorSet(1.f, 0.f, 0.f, 0.f);

                    vRightBase = XMVector3Normalize(vRightBase);

                    const _float fSide = CRandomUtil::Get_Float(-0.12f, 0.12f);
                    const _float fUp = CRandomUtil::Get_Float(0.03f, 0.10f);

                    _vector vDir = XMVector3Normalize(
                        vBack +
                        vRightBase * fSide +
                        XMVectorSet(0.f, fUp, 0.f, 0.f));

                    XMStoreFloat4(&pRuntime->vecDirections[i], XMVectorSetW(vDir, 0.f));
                }
                else if (pParticle->eSimulation == PARTICLE_SIMULATION::DRIFT)
                {
                    _vector vForward = XMLoadFloat3(&pData->vParticleForward);
                    if (XMVector3NearEqual(vForward, XMVectorZero(), XMVectorReplicate(0.0001f)))
                        vForward = XMVectorSet(0.f, 0.f, 1.f, 0.f);

                    vForward = XMVector3Normalize(vForward);

                    _vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
                    _vector vRightBase = XMVector3Cross(vWorldUp, -vForward);

                    if (XMVector3NearEqual(vRightBase, XMVectorZero(), XMVectorReplicate(0.0001f)))
                        vRightBase = XMVectorSet(1.f, 0.f, 0.f, 0.f);

                    vRightBase = XMVector3Normalize(vRightBase);

                    const _float fSide = CRandomUtil::Get_Float(-0.25f, 0.25f);
                    const _float fForward = CRandomUtil::Get_Float(-0.15f, 0.15f);
                    const _float fUp = CRandomUtil::Get_Float(0.35f, 0.75f);

                    _vector vDir = XMVector3Normalize(
                        vRightBase * fSide +
                        vForward * fForward +
                        XMVectorSet(0.f, fUp, 0.f, 0.f));

                    XMStoreFloat4(&pRuntime->vecDirections[i], XMVectorSetW(vDir, 0.f));
                }
                else
                {
                    pRuntime->vecDirections[i] = _float4(0.f, 0.f, 1.f, 0.f);
                }

                bAnyAlive = true;
            }
            else
            {
                /* 죽은 파티클은 화면 밖 혹은 0 스케일 처리 */
                pRuntime->vecInstances[i].vRight = _float4(0.f, 0.f, 0.f, 0.f);
                pRuntime->vecInstances[i].vUp = _float4(0.f, 0.f, 0.f, 0.f);
                pRuntime->vecInstances[i].vLook = _float4(0.f, 0.f, 1.f, 0.f);
            }
        }

        pInstance[i] = pRuntime->vecInstances[i];
    }

    m_pContext->Unmap(pRuntime->pInstanceVB.Get(), 0);

    if (!pParticle->isLoop && bAnyAlive == false)
    {
        pData->bParticlePlaying = false;
        pData->bParticleFinished = true;
    }
    else
    {
        pData->bParticleFinished = false;
    }
}

_bool CMeshRenderer_Processor::Create_ParticleBuffers(PARTICLE_RUNTIME* pRuntime)
{
    if (!pRuntime)
        return false;

    if (pRuntime->iNumInstances == 0 || pRuntime->iInstanceStride == 0)
        return false;

    VTXPOS vPoint{};
    vPoint.vPosition = _float3(0.f, 0.f, 0.f);

    D3D11_BUFFER_DESC vbDesc{};
    vbDesc.ByteWidth = sizeof(VTXPOS);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;
    vbDesc.MiscFlags = 0;
    vbDesc.StructureByteStride = sizeof(VTXPOS);

    D3D11_SUBRESOURCE_DATA vbData{};
    vbData.pSysMem = &vPoint;

    if (FAILED(m_pDevice->CreateBuffer(&vbDesc, &vbData, pRuntime->pPointVB.GetAddressOf())))
        return false;

    D3D11_BUFFER_DESC instDesc{};
    instDesc.ByteWidth = pRuntime->iInstanceStride * pRuntime->iNumInstances;
    instDesc.Usage = D3D11_USAGE_DYNAMIC;
    instDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    instDesc.MiscFlags = 0;
    instDesc.StructureByteStride = pRuntime->iInstanceStride;

    D3D11_SUBRESOURCE_DATA instData{};
    instData.pSysMem = pRuntime->vecInstances.data();

    if (FAILED(m_pDevice->CreateBuffer(&instDesc, &instData, pRuntime->pInstanceVB.GetAddressOf())))
        return false;

    return true;
}

_bool CMeshRenderer_Processor::Ensure_ParticleRuntime(MESH_RENDERER_DATA* pData)
{
    if (!pData)
        return false;

    if (pData->hParticle == INVALID_HANDLE_UINT)
        return false;

    if (pData->iParticleRuntime == INVALID_HANDLE_UINT)
        pData->iParticleRuntime = Allocate_ParticleRuntime();

    PARTICLE_RUNTIME* pRuntime = Get_ParticleRuntime(pData->iParticleRuntime);
    if (!pRuntime)
        return false;

    if (pRuntime->bInitialized)
        return true;

    PARTICLE_ENTRY* pParticle = SYS_RESOURCE.Get_Particle(pData->hParticle);
    if (!pParticle || !pParticle->Is_Valid())
    {
        Release_ParticleRuntime(pData->iParticleRuntime);
        pData->iParticleRuntime = INVALID_HANDLE_UINT;
        return false;
    }

    if (pParticle->hTexture == INVALID_HANDLE_UINT || pParticle->iMaxParticles == 0)
    {
        Release_ParticleRuntime(pData->iParticleRuntime);
        pData->iParticleRuntime = INVALID_HANDLE_UINT;
        return false;
    }

    pRuntime->iNumInstances = pParticle->iMaxParticles;
    pRuntime->vecInstances.resize(pRuntime->iNumInstances);
    pRuntime->vecSpeeds.resize(pRuntime->iNumInstances);
    pRuntime->vecDirections.resize(pRuntime->iNumInstances);
    pRuntime->hTexture = pParticle->hTexture;

    for (_uint i = 0; i < pRuntime->iNumInstances; ++i)
    {
        const _float fScaleX = CRandomUtil::Get_Float(pParticle->vScaleX.x, pParticle->vScaleX.y);
        const _float fScaleY = CRandomUtil::Get_Float(pParticle->vScaleY.x, pParticle->vScaleY.y);

        pRuntime->vecSpeeds[i] = CRandomUtil::Get_Float(pParticle->vSpeed.x, pParticle->vSpeed.y);

        pRuntime->vecInstances[i].vRight = _float4(fScaleX, 0.f, 0.f, 0.f);
        pRuntime->vecInstances[i].vUp = _float4(0.f, fScaleY, 0.f, 0.f);
        pRuntime->vecInstances[i].vLook = _float4(0.f, 0.f, 1.f, 0.f);

        pRuntime->vecInstances[i].vTranslation = _float4(
            CRandomUtil::Get_Float(pParticle->vCenter.x - pParticle->vRange.x * 0.5f, pParticle->vCenter.x + pParticle->vRange.x * 0.5f),
            CRandomUtil::Get_Float(pParticle->vCenter.y - pParticle->vRange.y * 0.5f, pParticle->vCenter.y + pParticle->vRange.y * 0.5f),
            CRandomUtil::Get_Float(pParticle->vCenter.z - pParticle->vRange.z * 0.5f, pParticle->vCenter.z + pParticle->vRange.z * 0.5f),
            1.f);

        pRuntime->vecInstances[i].vLifeTime = _float2(
            CRandomUtil::Get_Float(pParticle->vLifeTime.x, pParticle->vLifeTime.y),
            0.f);

        pRuntime->vecDirections[i] = _float4(0.f, 0.f, 1.f, 0.f);
    }

    if (!Create_ParticleBuffers(pRuntime))
    {
        Release_ParticleRuntime(pData->iParticleRuntime);
        pData->iParticleRuntime = INVALID_HANDLE_UINT;
        return false;
    }

    pRuntime->bInitialized = true;
    return true;
}

void CMeshRenderer_Processor::Reset_ParticleRuntime(MESH_RENDERER_DATA* pData)
{
    if (!pData)
        return;

    if (pData->eMode != MESH_MODE::PARTICLE)
        return;

    if (!Ensure_ParticleRuntime(pData))
        return;

    PARTICLE_RUNTIME* pRuntime = Get_ParticleRuntime(pData->iParticleRuntime);
    PARTICLE_ENTRY* pParticle = SYS_RESOURCE.Get_Particle(pData->hParticle);

    if (!pRuntime || !pParticle)
        return;

    _vector vForward = XMLoadFloat3(&pData->vParticleForward);
    if (XMVector3NearEqual(vForward, XMVectorZero(), XMVectorReplicate(0.0001f)))
        vForward = XMVectorSet(0.f, 0.f, 1.f, 0.f);

    vForward = XMVector3Normalize(vForward);

    _vector vBack = -vForward;
    _vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    _vector vRightBase = XMVector3Cross(vWorldUp, vBack);

    if (XMVector3NearEqual(vRightBase, XMVectorZero(), XMVectorReplicate(0.0001f)))
        vRightBase = XMVectorSet(1.f, 0.f, 0.f, 0.f);

    vRightBase = XMVector3Normalize(vRightBase);

    for (_uint i = 0; i < pRuntime->iNumInstances; ++i)
    {
        const _float fScaleX = CRandomUtil::Get_Float(pParticle->vScaleX.x, pParticle->vScaleX.y);
        const _float fScaleY = CRandomUtil::Get_Float(pParticle->vScaleY.x, pParticle->vScaleY.y);

        pRuntime->vecSpeeds[i] = CRandomUtil::Get_Float(pParticle->vSpeed.x, pParticle->vSpeed.y);

        pRuntime->vecInstances[i].vRight = _float4(fScaleX, 0.f, 0.f, 0.f);
        pRuntime->vecInstances[i].vUp = _float4(0.f, fScaleY, 0.f, 0.f);
        pRuntime->vecInstances[i].vLook = _float4(0.f, 0.f, 1.f, 0.f);

        if (pParticle->eSimulation == PARTICLE_SIMULATION::SLIDE)
        {
            /* 시작 위치도 슬라이딩 방향 기준으로 뿌린다 */
            const _float fSpawnSide = CRandomUtil::Get_Float(-0.04f, 0.04f);   // 좌우 아주 좁게
            const _float fSpawnBack = CRandomUtil::Get_Float(-0.4f, 0.4f);   // 약간 뒤쪽으로 길게
            const _float fSpawnUp = CRandomUtil::Get_Float(0.00f, 0.2f);      // 바닥에서 살짝만 띄우기

            _vector vSpawn =
                vBack * fSpawnBack +
                vRightBase * fSpawnSide +
                XMVectorSet(0.f, fSpawnUp, 0.f, 0.f);

            _float3 vSpawn3{};
            XMStoreFloat3(&vSpawn3, vSpawn);

            pRuntime->vecInstances[i].vTranslation = _float4(
                pParticle->vCenter.x + vSpawn3.x,
                pParticle->vCenter.y + vSpawn3.y,
                pParticle->vCenter.z + vSpawn3.z,
                1.f);

            /* 이동 방향은 거의 뒤쪽 직선, 좌우 흔들림은 아주 조금만 */
            const _float fSide = CRandomUtil::Get_Float(-0.06f, 0.06f);
            const _float fUp = CRandomUtil::Get_Float(0.01f, 0.06f);

            _vector vDir = XMVector3Normalize(
                vBack +
                vRightBase * fSide +
                XMVectorSet(0.f, fUp, 0.f, 0.f));

            XMStoreFloat4(&pRuntime->vecDirections[i], XMVectorSetW(vDir, 0.f));
        }
        else if (pParticle->eSimulation == PARTICLE_SIMULATION::DRIFT)
        {
            const _float fSpawnSide = CRandomUtil::Get_Float(-0.08f, 0.08f);
            const _float fSpawnForward = CRandomUtil::Get_Float(-0.08f, 0.08f);
            const _float fSpawnUp = CRandomUtil::Get_Float(-0.02f, 0.08f);

            _vector vSpawn =
                vRightBase * fSpawnSide +
                vForward * fSpawnForward +
                XMVectorSet(0.f, fSpawnUp, 0.f, 0.f);

            _float3 vSpawn3{};
            XMStoreFloat3(&vSpawn3, vSpawn);

            pRuntime->vecInstances[i].vTranslation = _float4(
                pParticle->vCenter.x + vSpawn3.x,
                pParticle->vCenter.y + vSpawn3.y,
                pParticle->vCenter.z + vSpawn3.z,
                1.f);

            const _float fSide = CRandomUtil::Get_Float(-0.25f, 0.25f);
            const _float fForward = CRandomUtil::Get_Float(-0.15f, 0.15f);
            const _float fUp = CRandomUtil::Get_Float(0.35f, 0.75f);

            _vector vDir = XMVector3Normalize(
                vRightBase * fSide +
                vForward * fForward +
                XMVectorSet(0.f, fUp, 0.f, 0.f));

            XMStoreFloat4(&pRuntime->vecDirections[i], XMVectorSetW(vDir, 0.f));
        }
        else if (pParticle->eSimulation == PARTICLE_SIMULATION::DUST)
        {
            const _float fSpawnSide = CRandomUtil::Get_Float(-0.18f, 0.18f);
            const _float fSpawnForward = CRandomUtil::Get_Float(-0.18f, 0.18f);
            const _float fSpawnUp = CRandomUtil::Get_Float(0.00f, 0.10f);

            _vector vSpawn =
                vRightBase * fSpawnSide +
                vForward * fSpawnForward +
                XMVectorSet(0.f, fSpawnUp, 0.f, 0.f);

            _float3 vSpawn3{};
            XMStoreFloat3(&vSpawn3, vSpawn);

            pRuntime->vecInstances[i].vTranslation = _float4(
                pParticle->vCenter.x + vSpawn3.x,
                pParticle->vCenter.y + vSpawn3.y,
                pParticle->vCenter.z + vSpawn3.z,
                1.f);

            pRuntime->vecDirections[i] = _float4(0.f, 0.f, 1.f, 0.f);
        }
        else
        {
            pRuntime->vecInstances[i].vTranslation = _float4(
                CRandomUtil::Get_Float(pParticle->vCenter.x - pParticle->vRange.x * 0.5f, pParticle->vCenter.x + pParticle->vRange.x * 0.5f),
                CRandomUtil::Get_Float(pParticle->vCenter.y - pParticle->vRange.y * 0.5f, pParticle->vCenter.y + pParticle->vRange.y * 0.5f),
                CRandomUtil::Get_Float(pParticle->vCenter.z - pParticle->vRange.z * 0.5f, pParticle->vCenter.z + pParticle->vRange.z * 0.5f),
                1.f);

            pRuntime->vecDirections[i] = _float4(0.f, 0.f, 1.f, 0.f);
        }

        pRuntime->vecInstances[i].vLifeTime = _float2(
            CRandomUtil::Get_Float(pParticle->vLifeTime.x, pParticle->vLifeTime.y),
            0.f);
    }

    pData->bParticleFinished = false;
}

uint32_t CMeshRenderer_Processor::Allocate_ParticleRuntime()
{
    if (!m_vecFreeParticleRuntime.empty())
    {
        const uint32_t iRuntime = m_vecFreeParticleRuntime.back();
        m_vecFreeParticleRuntime.pop_back();

        m_vecParticleRuntime[iRuntime] = PARTICLE_RUNTIME{};
        return iRuntime;
    }

    const uint32_t iRuntime = To<uint32_t>(m_vecParticleRuntime.size());
    m_vecParticleRuntime.push_back(PARTICLE_RUNTIME{});
    return iRuntime;
}

void CMeshRenderer_Processor::Release_ParticleRuntime(uint32_t iRuntime)
{
    if (iRuntime == INVALID_HANDLE_UINT || iRuntime >= m_vecParticleRuntime.size())
        return;

    m_vecParticleRuntime[iRuntime] = PARTICLE_RUNTIME{};
    m_vecFreeParticleRuntime.push_back(iRuntime);
}

PARTICLE_RUNTIME* CMeshRenderer_Processor::Get_ParticleRuntime(uint32_t iRuntime)
{
    if (iRuntime == INVALID_HANDLE_UINT || iRuntime >= m_vecParticleRuntime.size())
        return nullptr;

    return &m_vecParticleRuntime[iRuntime];
}

_bool CMeshRenderer_Processor::Update_Frustum()
{
    _float4x4 matView{};
    _float4x4 matProj{};

    matView = SYS_RENDER.Contexts()->Get_View();
    matProj = SYS_RENDER.Contexts()->Get_Proj();

    _matrix matViewXM = XMLoadFloat4x4(&matView);
    _matrix matProjXM = XMLoadFloat4x4(&matProj);

    _matrix matViewProjXM = matViewXM * matProjXM;

    XMStoreFloat4x4(&m_matViewProj, matViewProjXM);

    return true;
}

_bool CMeshRenderer_Processor::Try_Get_WorldMatrix(COMPONENT_HANDLE hTransform, _float4x4& matWorld) const
{
    if (!hTransform.Is_Valid())
        return false;
    TRANSFORM_DATA* pTransformData = To<TRANSFORM_DATA*>(m_pTransformProcessor->Get_DataPtr(COMPONENT_TYPE::TRANSFORM, hTransform));
    if (!pTransformData)
        return false;

    matWorld = pTransformData->matWorld;
    return true;
}

_bool CMeshRenderer_Processor::Is_Culled_By_Frustum(const MESH_ENTRY* pMesh, COMPONENT_HANDLE hTransform) const
{
    if (!pMesh)
        return false;

    _float4x4 matWorld{};
    if (!Try_Get_WorldMatrix(hTransform, matWorld))
        return false;

    const _float fSizeX = pMesh->maxAABB.x - pMesh->minAABB.x;
    const _float fSizeY = pMesh->maxAABB.y - pMesh->minAABB.y;
    const _float fSizeZ = pMesh->maxAABB.z - pMesh->minAABB.z;

    if (fSizeX <= 0.0001f || fSizeY <= 0.0001f || fSizeZ <= 0.0001f)
        return false;

    if (!isfinite(fSizeX) || !isfinite(fSizeY) || !isfinite(fSizeZ))
        return false;

    _float3 vCenter =
    {
        (pMesh->minAABB.x + pMesh->maxAABB.x) * 0.5f,
        (pMesh->minAABB.y + pMesh->maxAABB.y) * 0.5f,
        (pMesh->minAABB.z + pMesh->maxAABB.z) * 0.5f
    };

    _float3 vExtent =
    {
        fSizeX * 0.5f,
        fSizeY * 0.5f,
        fSizeZ * 0.5f
    };

    /*
        일부 잘림/깜빡임 방지용 안전 패딩.
    */
    const _float fPadding = -0.1f;

    vExtent.x += fPadding;
    vExtent.y += fPadding;
    vExtent.z += fPadding;

    _float3 vMin =
    {
        vCenter.x - vExtent.x,
        vCenter.y - vExtent.y,
        vCenter.z - vExtent.z
    };

    _float3 vMax =
    {
        vCenter.x + vExtent.x,
        vCenter.y + vExtent.y,
        vCenter.z + vExtent.z
    };

    const _vector vLocalCorners[8] =
    {
        XMVectorSet(vMin.x, vMin.y, vMin.z, 1.f),
        XMVectorSet(vMax.x, vMin.y, vMin.z, 1.f),
        XMVectorSet(vMin.x, vMax.y, vMin.z, 1.f),
        XMVectorSet(vMax.x, vMax.y, vMin.z, 1.f),

        XMVectorSet(vMin.x, vMin.y, vMax.z, 1.f),
        XMVectorSet(vMax.x, vMin.y, vMax.z, 1.f),
        XMVectorSet(vMin.x, vMax.y, vMax.z, 1.f),
        XMVectorSet(vMax.x, vMax.y, vMax.z, 1.f)
    };

    _matrix matWorldXM = XMLoadFloat4x4(&matWorld);
    _matrix matViewProjXM = XMLoadFloat4x4(&m_matViewProj);

    _matrix matWVP = matWorldXM * matViewProjXM;

    _bool bAllLeft = true;
    _bool bAllRight = true;
    _bool bAllBottom = true;
    _bool bAllTop = true;
    _bool bAllNear = true;
    _bool bAllFar = true;

    for (_uint i = 0; i < 8; ++i)
    {
        _vector vClip = XMVector4Transform(vLocalCorners[i], matWVP);

        const _float x = XMVectorGetX(vClip);
        const _float y = XMVectorGetY(vClip);
        const _float z = XMVectorGetZ(vClip);
        const _float w = XMVectorGetW(vClip);

        if (fabsf(w) <= 0.0001f)
            return false;

        const _float fBias = fabsf(w) * 0.02f;

        // 화면 안이라면 아래 조건 충족
        //  -w <= x <= w
        //  -w <= y <= w
        //  0 <= z <= w

        if (x >= -w - fBias) bAllLeft = false;
        if (x <= w + fBias) bAllRight = false;

        if (y >= -w - fBias) bAllBottom = false;
        if (y <= w + fBias) bAllTop = false;

        if (z >= 0.f - fBias) bAllNear = false;
        if (z <= w + fBias) bAllFar = false;
    }

    if (bAllLeft || bAllRight || bAllBottom || bAllTop || bAllNear || bAllFar)
        return true;

    return false;
}

_bool CMeshRenderer_Processor::Is_Culled_Renderer_By_Frustum(const MESH_RENDERER_DATA* pData) const
{
    if (!pData)
        return false;

    if (pData->hMesh == INVALID_HANDLE_UINT)
        return false;

    if (pData->layer == RENDER_LAYER::SKY)
        return false;

    if (pData->eMode == MESH_MODE::PARTICLE)
        return false;

    /*
        애니메이션 / 스키닝 / 어태치는 일단 안전하게 컬링 제외
    */
    if (pData->hAnimator.Is_Valid() ||
        pData->hSkinningSourceAnimator.Is_Valid() ||
        pData->hAttachSourceAnimator.Is_Valid())
    {
        return false;
    }

    if (SYS_RESOURCE.Is_ModelHandle(pData->hMesh))
    {
        MODEL_ENTRY* pModel = SYS_RESOURCE.Get_Model(pData->hMesh);
        if (!pModel)
            return false;

        _bool bHasValidPart = false;

        for (const auto& pPart : pModel->parts)
        {
            MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(pPart.hMesh);
            if (!pMesh)
                continue;

            bHasValidPart = true;

            /*
                파트 하나라도 프러스텀에 걸리면 모델 전체를 살림.
            */
            if (!Is_Culled_By_Frustum(pMesh, pData->hTransform))
                return false;
        }

        /*
            유효 파트가 없으면 괜히 사라지게 하지 않음.
        */
        if (!bHasValidPart)
            return false;

        /*
            모든 파트가 프러스텀 밖이면 모델 컬링.
        */
        return true;
    }

    MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(pData->hMesh);
    if (!pMesh)
        return false;

    return Is_Culled_By_Frustum(pMesh, pData->hTransform);
}

std::unique_ptr<CMeshRenderer_Processor> CMeshRenderer_Processor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    auto pInstance = std::make_unique<CMeshRenderer_Processor>(pDevice, pContext);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}
