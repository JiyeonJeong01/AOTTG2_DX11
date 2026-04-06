#include "Debug_Renderer.h"

#include "CRender_System.h"
#include "Render_Context.h"

#include "DebugDraw.h"

NS_BEGIN(Engine)

CDebug_Renderer::CDebug_Renderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice(pDevice)
    , m_pContext(pContext)
{
}

CDebug_Renderer::~CDebug_Renderer()
{
}

HRESULT CDebug_Renderer::Initialize()
{
    if (m_pDevice == nullptr || m_pContext == nullptr)
        return E_FAIL;

    m_pBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(m_pContext);
    m_pEffect = std::make_unique<BasicEffect>(m_pDevice);
    m_pEffect->SetVertexColorEnabled(true);

    const void* pShaderByteCode = nullptr;
    size_t iByteCodeLength = 0;

    m_pEffect->GetVertexShaderBytecode(&pShaderByteCode, &iByteCodeLength);

    HRESULT hr = m_pDevice->CreateInputLayout(
        VertexPositionColor::InputElements,
        VertexPositionColor::InputElementCount,
        pShaderByteCode,
        iByteCodeLength,
        m_pInputLayout.GetAddressOf());

    if (FAILED(hr))
        return hr;

    return S_OK;
}

void CDebug_Renderer::Begin()
{
    if (m_bBegun || m_pContext == nullptr || m_pBatch == nullptr || m_pEffect == nullptr)
        return;

    m_pContext->IASetInputLayout(m_pInputLayout.Get());

    m_pEffect->SetWorld(XMMatrixIdentity());
    m_pEffect->SetView(Math::Load(SYS_RENDER.Contexts()->Get_View()));
    m_pEffect->SetProjection(Math::Load(SYS_RENDER.Contexts()->Get_Proj()));
    m_pEffect->Apply(m_pContext);

    m_pBatch->Begin();
    m_bBegun = true;
}

void CDebug_Renderer::End()
{
    if (!m_bBegun || m_pBatch == nullptr)
        return;

    m_pBatch->End();
    m_bBegun = false;
}

void CDebug_Renderer::Draw_Collider(const COLLIDER_PROXY_DATA& tProxy)
{
    if (!m_bBegun || tProxy.pCol == nullptr)
        return;

    switch (tProxy.pCol->eShape)
    {
    case SHAPE::BOX:
        Draw_Box(tProxy);
        break;

    case SHAPE::SPHERE:
        Draw_Sphere(tProxy);
        break;

    case SHAPE::PLANE:
        Draw_Plane(tProxy);
        break;

    default:
        break;
    }
}

void CDebug_Renderer::Draw_Box(const COLLIDER_PROXY_DATA& tProxy)
{
    Draw_AABB(tProxy.aabbWorld);
}

void CDebug_Renderer::Draw_Sphere(const COLLIDER_PROXY_DATA& tProxy)
{
    BoundingSphere tSphere{};
    tSphere.Center = tProxy.vCenterWorld;
    tSphere.Radius = tProxy.sphere.fRadiusWorld;

    DX::Draw(m_pBatch.get(), tSphere, DirectX::Colors::Lime);
}

void CDebug_Renderer::Draw_Plane(const COLLIDER_PROXY_DATA& tProxy)
{
    const XMVECTOR vOrigin = XMLoadFloat3(&tProxy.vCenterWorld);
    const XMVECTOR vAxisU = XMLoadFloat3(&tProxy.plane.vAxisUWorld);
    const XMVECTOR vAxisV = XMLoadFloat3(&tProxy.plane.vAxisVWorld);
    const XMVECTOR vNormal = XMLoadFloat3(&tProxy.plane.vNormalWorld);

    if (tProxy.plane.bInfinite)
    {
        DX::DrawGrid(
            m_pBatch.get(),
            XMVector3Normalize(vAxisU) * 5.f,
            XMVector3Normalize(vAxisV) * 5.f,
            vOrigin,
            10,
            10,
            Colors::Cyan);
    }
    else
    {
        const _float fHalfU = tProxy.plane.vDimension.x * 0.5f;
        const _float fHalfV = tProxy.plane.vDimension.y * 0.5f;

        DX::DrawGrid(
            m_pBatch.get(),
            XMVector3Normalize(vAxisU) * fHalfU,
            XMVector3Normalize(vAxisV) * fHalfV,
            vOrigin,
            2,
            2,
            Colors::Cyan);
    }

    DX::DrawRay(
        m_pBatch.get(),
        vOrigin,
        XMVector3Normalize(vNormal) * 1.0f,
        false,
        Colors::Red);
}

void CDebug_Renderer::Draw_AABB(const AABB& aabb)
{
    BoundingBox tBox{};
    tBox.Center = _float3(
        (aabb.vMin.x + aabb.vMax.x) * 0.5f,
        (aabb.vMin.y + aabb.vMax.y) * 0.5f,
        (aabb.vMin.z + aabb.vMax.z) * 0.5f);

    tBox.Extents = _float3(
        (aabb.vMax.x - aabb.vMin.x) * 0.5f,
        (aabb.vMax.y - aabb.vMin.y) * 0.5f,
        (aabb.vMax.z - aabb.vMin.z) * 0.5f);

    DX::Draw(m_pBatch.get(), tBox, Colors::Yellow);
}

void CDebug_Renderer::Draw_AABB(const AABB& aabb, FXMVECTOR vColor)
{
    BoundingBox tBox{};
    tBox.Center = _float3(
        (aabb.vMin.x + aabb.vMax.x) * 0.5f,
        (aabb.vMin.y + aabb.vMax.y) * 0.5f,
        (aabb.vMin.z + aabb.vMax.z) * 0.5f);

    tBox.Extents = _float3(
        (aabb.vMax.x - aabb.vMin.x) * 0.5f,
        (aabb.vMax.y - aabb.vMin.y) * 0.5f,
        (aabb.vMax.z - aabb.vMin.z) * 0.5f);

    DX::Draw(m_pBatch.get(), tBox, vColor);
}

void CDebug_Renderer::Draw_Line(const _float3& vStart, const _float3& vEnd, FXMVECTOR vColor)
{
    if (!m_bBegun || !m_pBatch)
        return;

    DirectX::VertexPositionColor v0;
    v0.position = { vStart.x, vStart.y, vStart.z };
    XMStoreFloat4(&v0.color, vColor);

    DirectX::VertexPositionColor v1;
    v1.position = { vEnd.x, vEnd.y, vEnd.z };
    XMStoreFloat4(&v1.color, vColor);

    m_pBatch->DrawLine(v0, v1);
}

void CDebug_Renderer::Draw_NavCell(const _float3& vA, const _float3& vB, const _float3& vC, FXMVECTOR vColor)
{
    if (!m_bBegun || !m_pBatch)
        return;

    DX::DrawTriangle(
        m_pBatch.get(),
        XMLoadFloat3(&vA),
        XMLoadFloat3(&vB),
        XMLoadFloat3(&vC),
        vColor);
}

void CDebug_Renderer::Draw_NavPoint(const _float3& vPos, _float fSize, FXMVECTOR vColor)
{
    if (!m_bBegun || !m_pBatch)
        return;

    const _float3 vLeft = { vPos.x - fSize, vPos.y, vPos.z };
    const _float3 vRight = { vPos.x + fSize, vPos.y, vPos.z };
    const _float3 vDown = { vPos.x, vPos.y, vPos.z - fSize };
    const _float3 vUp = { vPos.x, vPos.y, vPos.z + fSize };

    Draw_Line(vLeft, vRight, vColor);
    Draw_Line(vDown, vUp, vColor);
}

std::unique_ptr<CDebug_Renderer> CDebug_Renderer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    std::unique_ptr<CDebug_Renderer> pInstance = std::unique_ptr<CDebug_Renderer>(new CDebug_Renderer(pDevice, pContext));

    if (FAILED(pInstance->Initialize()))
        return nullptr;

    return pInstance;
}

NS_END
