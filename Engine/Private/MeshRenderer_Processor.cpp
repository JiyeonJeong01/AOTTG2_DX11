#include "MeshRenderer_Processor.h"
#include "Engine_Log.h"
#include "Resource_System.h"
#include "Component_Spec.h"
#include "Transform_Processor.h"
#include "Engine_Math.h"
#include "GameObject.h"
#include "Texture.h"

CMeshRenderer_Processor::CMeshRenderer_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CTransform_Processor* pTransformProcessor)
    : m_pDevice(pDevice), m_pContext(pContext), m_pTransformProcessor(pTransformProcessor)
{
}

CMeshRenderer_Processor::~CMeshRenderer_Processor()
{
}
D3D11_RASTERIZER_DESC rd{};
D3D11_DEPTH_STENCIL_DESC ds{};
ID3D11RasterizerState* g_rsCullNone = nullptr;
ID3D11DepthStencilState* g_dsOff = nullptr;


HRESULT CMeshRenderer_Processor::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_pTransformProcessor, E_FAIL, "Transform Processor is nullptr");

    SYS_COMPONENT.Register_InitialSpecFactory<CMeshRenderer, MESH_RENDERER_SPEC>(COMPONENT_TYPE::MESH_RENDERER);
    SYS_COMPONENT.Register_BuildSpecFacotry<CMeshRenderer>(COMPONENT_TYPE::MESH_RENDERER);
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

            DRAW_CMD tCmd = DRAW_CMD::Create_Mesh(pData->hMesh, pData->hMaterial, pData->hMainTex, pData->hTransform, pData->flags, 0, 0);
            tCmd.sortKey = Make_SortKey(*pData);
            outCmds.push_back(tCmd);
        }
    }
}

HRESULT CMeshRenderer_Processor::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec)
{
    MESH_RENDERER_DATA* pData = m_Pool.Get_Data_By_Handle(handle);
    _DEBUG_ENGINE_ASSERT_MSG(pData != nullptr, "Invalid MeshRenderer handle in Initialize_From_Spec");

    pData->hMesh = SYS_RESOURCE.Load_Mesh(SCAST(const MESH_RENDERER_SPEC*, pSpec)->meshGUID);
    pData->hMaterial = SYS_RESOURCE.Load_Material_Temp(SCAST(const MESH_RENDERER_SPEC*, pSpec)->materialGUID, 0);
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

    CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);

    pData->hTransform = pObj->Get_Component<CTransform>(COMPONENT_TYPE::TRANSFORM).Get_Handle();
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

std::unique_ptr<CMeshRenderer_Processor> CMeshRenderer_Processor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CTransform_Processor* pTransform)
{
    auto pInstance = std::make_unique<CMeshRenderer_Processor>(pDevice, pContext, pTransform);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}
