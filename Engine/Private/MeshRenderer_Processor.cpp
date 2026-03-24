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

            COMPONENT_HANDLE hSkinningAnimator = INVALID_HANDLE;

            if (pData->hAnimator.Is_Valid())
                hSkinningAnimator = pData->hAnimator;
            else if (pData->hSkinningSourceAnimator.Is_Valid())
                hSkinningAnimator = pData->hSkinningSourceAnimator;

            DRAW_CMD tCmd = DRAW_CMD::Create_Mesh(
                pData->hMesh,
                pData->hMaterial,
                pData->hTransform,
                hSkinningAnimator,
                pData->flags,
                0,
                0);

            tCmd.mesh.hPerObjectParams = pData->hPerObjectParams;
            tCmd.sortKey = Make_SortKey(*pData);

            if (pData->hSkinningSourceAnimator.Is_Valid())
                if (Build_SkinnedPart_BoneMatrices(pData))
                    tCmd.mesh.pSkinningMatrices = &pData->vecSkinningBoneMatrices;

            outCmds.push_back(tCmd);
        }
    }
}

HRESULT CMeshRenderer_Processor::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec)
{
    MESH_RENDERER_DATA* pData = m_Pool.Get_Data_By_Handle(handle);
    _DEBUG_ENGINE_ASSERT_MSG(pData != nullptr, "Invalid MeshRenderer handle in Initialize_From_Spec");

    pData->hMesh = SYS_RESOURCE.Load_Mesh(SCAST(const MESH_RENDERER_SPEC*, pSpec)->meshGUID);
    pData->hMaterial = SYS_RESOURCE.Load_Material(SCAST(const MESH_RENDERER_SPEC*, pSpec)->materialGUID);
    pData->flags = SCAST(const MESH_RENDERER_SPEC*, pSpec)->flags;
    pData->layer = SCAST(const MESH_RENDERER_SPEC*, pSpec)->layer;
    pData->sortZ = SCAST(const MESH_RENDERER_SPEC*, pSpec)->sortZ;

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
    pData->hSkinningSourceAnimator = INVALID_HANDLE;

    CGameObject* pParent = pObj->Get_Parent();  /* 부모가 있는 경우 */
    if (pParent != nullptr)
    {
        CAnimator anim = pParent->Get_Component<CAnimator>();
        if (anim.Is_Valid())
        {
            pData->hSkinningSourceAnimator = anim.Get_Handle();
            pData->hTransform = pParent->Get_Component<CTransform>().Get_Handle();
        }
    }
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

    for (uint32_t i = 0; i < static_cast<uint32_t>(pData->vecSkinningBoneRemap.size()); ++i)
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

std::unique_ptr<CMeshRenderer_Processor> CMeshRenderer_Processor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    auto pInstance = std::make_unique<CMeshRenderer_Processor>(pDevice, pContext);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}
