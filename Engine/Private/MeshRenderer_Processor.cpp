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

}

void CMeshRenderer_Processor::LateUpdate(_float fDT)
{
}

void CMeshRenderer_Processor::Build_RenderQueue(vector<DRAW_CMD>& outCmds)
{
    if (!m_pTransformProcessor /* || !m_pCam */)
        return;
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
            if (!pData || !pData->bEnable) continue;
            if (pData->hMesh == 0 || pData->hMaterial == 0)
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

            if (pData->hSkinningSourceAnimator.Is_Valid())
            {
                if (Build_SkinnedPart_BoneMatrices(pData))
                    tCmd.mesh.pSkinningMatrices = &pData->vecSkinningBoneMatrices;
            }
            else if (pData->hAttachSourceAnimator.Is_Valid())
            {
                if (Build_Attach_BoneMatrix(pData))
                {
                    tCmd.mesh.matAttach = pData->matFinalAttach;
                }
            }

            outCmds.push_back(tCmd);
        }
    }
}

HRESULT CMeshRenderer_Processor::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec)
{
    MESH_RENDERER_DATA* pData = m_Pool.Get_Data_By_Handle(handle);
    _DEBUG_ENGINE_ASSERT_MSG(pData != nullptr, "Invalid MeshRenderer handle in Initialize_From_Spec");

    const auto* pMeshSpec = SCAST(const MESH_RENDERER_SPEC*, pSpec);

    pData->hMesh = SYS_RESOURCE.Load_Mesh(pMeshSpec->meshGUID);
    pData->hMaterial = SYS_RESOURCE.Load_Material(pMeshSpec->materialGUID);
    pData->flags = pMeshSpec->flags;
    pData->layer = pMeshSpec->layer;
    pData->sortZ = pMeshSpec->sortZ;
    pData->bEnable = pMeshSpec->bEnable;

    pData->eMode = pMeshSpec->eMode;
    pData->strAttachBoneName = pMeshSpec->strAttachBoneName;

    if (pData->eMode == MESH_MODE::PARTS)
        return Resolve_SkinningReference(handle) ? S_OK : E_FAIL;
    if (pData->eMode == MESH_MODE::ATTACH)
        return Resolve_AttachReference(handle) ? S_OK : E_FAIL;

    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE>
CMeshRenderer_Processor::Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    IF_TRUE_RETURN_MSG_BREAK(eComType != COMPONENT_TYPE::MESH_RENDERER, nullptr, "Wrong component type.");

    MESH_RENDERER_DATA* pData = m_Pool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, nullptr, "Invalid MeshRenderer handle in Build_Spec");

    auto spec = std::make_unique<MESH_RENDERER_SPEC>();

    if (SYS_RESOURCE.Is_ModelHandle(pData->hMesh))
        spec->meshGUID = SYS_RESOURCE.Get_Model(pData->hMesh)->tGUID;
    else
        spec->meshGUID = SYS_RESOURCE.Get_Mesh(pData->hMesh)->tGUID;

    spec->materialGUID = SYS_RESOURCE.Get_Material(pData->hMaterial)->tGUID;
    spec->flags = pData->flags;
    spec->layer = pData->layer;
    spec->sortZ = pData->sortZ;
    spec->bEnable = pData->bEnable;

    spec->eMode = pData->eMode;
    spec->strAttachBoneName = pData->strAttachBoneName;

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

std::unique_ptr<CMeshRenderer_Processor> CMeshRenderer_Processor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    auto pInstance = std::make_unique<CMeshRenderer_Processor>(pDevice, pContext);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}
