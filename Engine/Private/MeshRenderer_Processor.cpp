#include "MeshRenderer_Processor.h"
#include "Engine_Log.h"
#include "Resource_System.h"
#include "Component_Spec.h"
#include "Transform_Processor.h"
#include "Core_System.h"

CMeshRenderer_Processor::CMeshRenderer_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CTransform_Processor* pTransformProcessor)
    : m_pDevice(pDevice), m_pContext(pContext), m_pTransformProcessor(pTransformProcessor)
{
}

CMeshRenderer_Processor::~CMeshRenderer_Processor()
{
}

HRESULT CMeshRenderer_Processor::Initialize()
{
    _DEBUG_NULL_BREAK_RETURN_MSG(m_pTransformProcessor, E_FAIL, "Transform Processor is nullptr");
    return S_OK;
}

void CMeshRenderer_Processor::Update(_float fDT)
{
    Immediate_Render_All();

}

void CMeshRenderer_Processor::LateUpdate(_float fDT)
{
}

void CMeshRenderer_Processor::Begin_Frame()
{
    /* 카메라 */
}

void CMeshRenderer_Processor::End_Frame()
{
    /* 카메라 해제 */
}

void CMeshRenderer_Processor::Build_Queue(std::vector<DRAW_CMD>& outCmds)
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
            if (!pPage->Is_Active(i))
                continue;
            auto* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnabled) continue;
            //if (pData->hMesh == 0 || pData->hMaterial == 0)
            //    continue;

            DRAW_CMD cmd{};
            cmd.hMesh = pData->hMesh;
            cmd.hMaterial = pData->hMaterial;
            cmd.hTransform = pData->hTransform;
            cmd.flags = pData->flags;
            cmd.firstIndex = 0;
            cmd.indexCount = 0;
            cmd.sortKey = Make_SortKey(cmd, *pData);
            outCmds.push_back(cmd);
        }
    }
}

void CMeshRenderer_Processor::Immediate_Render_All()
{
    SYS_CORE.Bind_SceneRT();

    if (!m_pTransformProcessor /* || !m_pCam */)
        return;
    std::vector<DRAW_CMD> cmds;
    cmds.reserve(64);
    Build_Queue(cmds);

    std::sort(cmds.begin(), cmds.end(), [](const DRAW_CMD& a, const DRAW_CMD& b)
        {
            return a.sortKey < b.sortKey;
        });

    for (auto& cmd : cmds)
        Execute_Draw(cmd);
}

HRESULT CMeshRenderer_Processor::Initialize_From_Spec(COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec)
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

uint64_t CMeshRenderer_Processor::Make_SortKey(const DRAW_CMD& cmd, const MESH_RENDERER_DATA& tData) const
{
    /* [63:60] layer (4 bit)*/
    /* [59:28] material(32 bit) */
    /* [27:0 ] mesh(28 bit) */
    uint64_t key = 0;
    key |= (uint64_t)((uint8_t)tData.layer & 0xF) << 60;
    key |= (uint64_t)(cmd.hMaterial) << 28;
    key |= (uint64_t)(cmd.hMesh & 0x0FFFFFFF);
    return key;
}

void CMeshRenderer_Processor::Execute_Draw(const DRAW_CMD& cmd)
{
    const MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(cmd.hMesh);
    MATERIAL_ENTRY* pMat = SYS_RESOURCE.Get_Material(cmd.hMaterial);

    _DEBUG_NULL_BREAK_RETURN_MSG(pMesh, , "Mesh is nullptr.");
    _DEBUG_NULL_BREAK_RETURN_MSG(pMat, , "Material is nullptr.");

    const SHADER_ENTRY* pShader = SYS_RESOURCE.Get_Shader(pMat->hShader);
    _DEBUG_NULL_BREAK_RETURN_MSG(pShader, , "Shader is nullptr.");

    const uint16_t passIndex = pMat->passIndex;
    if (passIndex >= pShader->pPasses.size())
        return;

    ///* Set matrix */
    //const _matrix matWorld = XMMatrixIdentity(); // (0, 0, 0) 위치, 회전 없음, 크기 1
    //// const _matrix matWorld = m_pTransformProcessor->Get_Proxy(cmd.hTransform).Get_WorldXM();
    //const _matrix matView = XMMatrixTranslation(0, 0, 5.0f);
    //const _matrix matProj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 16.0f / 9.0f, 0.01f, 1000.0f);
    //pMat->pWorld->SetMatrix(reinterpret_cast<const float*>(&matWorld));
    //pMat->pView->SetMatrix(reinterpret_cast<const float*>(&matView));
    //pMat->pProj->SetMatrix(reinterpret_cast<const float*>(&matProj));

    const _matrix matWorld = XMMatrixIdentity();

    // 2. 뷰: 카메라를 (0, 0, -5)에 두고 (0, 0, 0)을 바라보게 함 (가장 안전한 방법)
    const _vector vEye = XMVectorSet(0.f, 0.f, -5.f, 0.f);
    const _vector vAt = XMVectorSet(0.f, 0.f, 0.f, 0.f);
    const _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    const _matrix matView = XMMatrixLookAtLH(vEye, vAt, vUp);

    // 3. 투영: 시야각 90도
    const _matrix matProj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 16.0f / 9.0f, 0.01f, 1000.0f);

    /* ★ 핵심: HLSL 전달을 위해 전치(Transpose) 수행 ★ */
    _matrix matW = XMMatrixTranspose(matWorld);
    _matrix matV = XMMatrixTranspose(matView);
    _matrix matP = XMMatrixTranspose(matProj);

    // 전치된 행렬을 넣어줘야 셰이더가 올바르게 계산합니다.
    pMat->pWorld->SetMatrix(reinterpret_cast<const float*>(&matW));
    pMat->pView->SetMatrix(reinterpret_cast<const float*>(&matV));
    pMat->pProj->SetMatrix(reinterpret_cast<const float*>(&matP));

    /* TODO : Set Texture*/

    /* Apply */
    ID3D11InputLayout* pIL = pShader->pPasses[passIndex].pInputLayout.Get();
    m_pContext->IASetInputLayout(pIL);

    ID3DX11EffectPass* pPass = pShader->pPasses[passIndex].pPass;
    if (!pPass)
        return;
    pPass->Apply(0, m_pContext);

    /* Mesh bingind and draw call */
    pMesh->Bind_IA(m_pContext);
    pMesh->Draw(m_pContext, cmd.firstIndex, cmd.indexCount);
}

std::unique_ptr<CMeshRenderer_Processor> CMeshRenderer_Processor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CTransform_Processor* pTransform)
{
    auto pInstance = std::make_unique<CMeshRenderer_Processor>(pDevice, pContext, pTransform);

    if (FAILED(pInstance->Initialize()))
    {
        _DEBUG_ERROR_BREAK("Create instance failed");
        return nullptr;
    }

    return pInstance;
}
