#include "Line.h"

#include "MeshBuilder.h"
#include "Resource_System.h"
#include "CRender_System.h"
#include "Render_Context.h"

#include "Engine_Math.h"

CLine::CLine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice(pDevice), m_pContext(pContext)
{
}

CLine::~CLine()
{
}

HRESULT CLine::Initialize(_uint iMaxPoints, _float fThickness)
{
    m_iMaxPoints = iMaxPoints;
    m_fThickness = fThickness;

    MESH_ENTRY entry{};
    if (FAILED(CMeshBuilder::Create_RibbonLine_VtxCol(m_pDevice, entry, m_iMaxPoints)))
        return E_FAIL;

    m_hMesh = SYS_RESOURCE.Register_MeshEntry(std::move(entry));
    if (m_hMesh == INVALID_HANDLE_UINT)
        return E_FAIL;

    /* 실패 검사 */
    IF_NULL_RETURN_MSG_BREAK(m_pDevice, E_FAIL, "device is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_pContext, E_FAIL, "context is nullptr");
    IF_TRUE_RETURN_MSG_BREAK(m_iMaxPoints < 2, E_FAIL, "iMaxPoints must be over two");
    IF_TRUE_RETURN_MSG_BREAK(m_fThickness < 0.f, E_FAIL, "iThickness must be over zero");

    return S_OK;
}

HRESULT CLine::Update(const _float3* pPoints, _uint iNumPoints)
{
    if (!pPoints || m_hMesh == INVALID_HANDLE_UINT)
        return E_FAIL;
    if (iNumPoints < 2 || iNumPoints > m_iMaxPoints)
        return E_FAIL;

    MESH_ENTRY* pEntry = SYS_RESOURCE.Get_Mesh(m_hMesh);
    if (!pEntry)
        return E_FAIL;

    ID3D11Buffer* pVB = *pEntry->pVB.GetAddressOf();
    if (!pVB)
        return E_FAIL;

    /* CPU -> GPU 접근 */
    D3D11_MAPPED_SUBRESOURCE mapped{};

    /* CPU가 GPU VRAM에 직접 쓰는 게 아닌 CPU가 접근 가능한 임시 메모리 주소를 mapped.data에 담아 반환한다.*/
    if (FAILED(m_pContext->Map(pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        return E_FAIL;

    /* void* -> VTXCOL* 캐스팅 */
    VTXCOL* pVertices = To<VTXCOL*>(mapped.pData);

    const _float fHalfThickness = m_fThickness * 0.5f;

    const _float3 vCamPos = SYS_RENDER.Contexts()->Get_CamPosition();
    const _vector vCamPosVec = Math::Load(vCamPos);

    for (_uint i = 0; i < iNumPoints; ++i)
    {
        const _vector vPoint = Math::Load(pPoints[i]);

        _vector vPrev = vPoint;
        _vector vNext = vPoint;

        if (i == 0)
        {
            vNext = Math::Load(pPoints[i + 1]);
        }
        else if (i == iNumPoints - 1)
        {
            vPrev = Math::Load(pPoints[i - 1]);
        }
        else
        {
            vPrev = Math::Load(pPoints[i - 1]);
            vNext = Math::Load(pPoints[i + 1]);
        }

        /* 리본이 나아가는 진행 방향 */
        _vector vTangent = vNext - vPrev;

        const _float fEps = 1e-6f;
        if (Math::Get_X(XMVector3LengthSq(vTangent)) < fEps)
            vTangent = { 0.f, 1.f, 0.f, 0.f };

        vTangent = Math::Normalize(vTangent);

        _vector vPointToCamera = vCamPosVec - vPoint;
        if (Math::Get_X(XMVector3LengthSq(vPointToCamera)) < fEps)
            vPointToCamera = { 0.f, 0.f, 1.f, 0.f };

        vPointToCamera = Math::Normalize(vPointToCamera);

        /* vSide : 두께를 넓히는 방향 */
        _vector vSide = Math::Cross(vPointToCamera, vTangent);
        if (Math::Get_X(XMVector3LengthSq(vSide)) < fEps)
            vSide = Math::Cross(Math::Set_Vec(0.f, 1.f, 0.f, 0.f), vTangent);

        if (Math::Get_X(XMVector3LengthSq(vSide)) < fEps)
            vSide = Math::Set_Vec(1.f, 0.f, 0.f, 0.f);

        /* 두께 벡터 */
        vSide = Math::Normalize(vSide) * fHalfThickness;

        _float3 vLeft{}, vRight{};

        /* 선이 그려질 너비 */
        Math::Store(vLeft, vPoint - vSide);
        Math::Store(vRight, vPoint + vSide);

        pVertices[i * 2 + 0].vPosition = vLeft;
        pVertices[i * 2 + 0].vColor = m_vColor;

        pVertices[i * 2 + 1].vPosition = vRight;
        pVertices[i * 2 + 1].vColor = m_vColor;
    }

    /* 제어권 반환 */
    m_pContext->Unmap(pVB, 0);

    m_iCurPoints = iNumPoints;

    pEntry->iVertexCount = iNumPoints * 2;
    pEntry->iIndexCount = (iNumPoints - 1) * 6;

    return S_OK;
}

void CLine::Submit()
{
    if (m_hMesh == INVALID_HANDLE_UINT)
        return;
    if (m_iCurPoints < 2)
        return;

    MESH_ENTRY* pEntry = SYS_RESOURCE.Get_Mesh(m_hMesh);
    if (!pEntry)
        return;

    pEntry->iVertexCount = m_iCurPoints;

    DRAW_CMD cmd = DRAW_CMD::Create_Line(m_hMesh, DRAW_TYPE::LINE, RENDER_LAYER::NONBLEND);
    SYS_RENDER.Submit_LineMesh(cmd);
}

std::unique_ptr<CLine> CLine::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumPoints, _float fThickness)
{
    auto pInstance = std::make_unique<CLine>(pDevice, pContext);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(iNumPoints, fThickness), nullptr, "instnace create failed");
    return pInstance;
}
