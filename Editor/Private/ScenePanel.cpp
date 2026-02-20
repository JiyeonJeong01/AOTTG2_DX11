#include "ScenePanel.h"
#include "Core_System.h"
#include "HierarchyPanel.h"
#include "Gizmo.h"

#include "Transform.h"
#include "CRender_System.h"

#include "GameObject.h"

NS_BEGIN(Editor)

CScenePanel::CScenePanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName)
{
}

CScenePanel::~CScenePanel() = default;

HRESULT CScenePanel::Initialize(CHierarchyPanel* pHierarchy)
{
    pHierarchy->m_OnPrimarySelectionChanged.Add_Listener(&CScenePanel::Set_Target, this);

    m_pGizmo = CGizmo::Create();
    return S_OK;
}

void CScenePanel::Update()
{


}

void CScenePanel::Render()
{

    if (!ImGui::Begin(m_strPanelName.c_str()))
    {
        ImGui::End();
    }

    Draw_Toolbar();
    Draw_Viewport();

    ImGui::End();
}

void CScenePanel::Draw_Toolbar()
{
    if (m_pTarget)
    {
        if (ImGui::RadioButton("Translate", m_pGizmo->Get_Mode() == GIZMO_MODE::TRANSLATE))
            m_pGizmo->Set_Mode(GIZMO_MODE::TRANSLATE);
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", m_pGizmo->Get_Mode() == GIZMO_MODE::ROTATE))
            m_pGizmo->Set_Mode(GIZMO_MODE::ROTATE);
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", m_pGizmo->Get_Mode() == GIZMO_MODE::SCALE))
            m_pGizmo->Set_Mode(GIZMO_MODE::SCALE);
    }

    ImGui::Separator();
}

void CScenePanel::Draw_Viewport()
{
    ImVec2 avail = ImGui::GetContentRegionAvail();
    _uint w = (_uint)((avail.x > 0.f) ? avail.x : 0.f);
    _uint h = (_uint)((avail.y > 0.f) ? avail.y : 0.f);

    m_bHovered = ImGui::IsWindowHovered();
    m_bFocused = ImGui::IsWindowFocused();

    if (w == 0 || h == 0)
    {
        ImGui::TextDisabled("Viewport size is zero.");
        return;
    }

    Ensure_RenderTarget(w, h);

    // 이미지 출력
    ImGui::Image((ImTextureID)m_pSceneSRV, avail);

    // 뷰포트 rect 계산
    ImVec2 vpPos = ImGui::GetItemRectMin();
    ImVec2 vpSize = ImGui::GetItemRectSize();

    // 선택 없으면 끝
    if (!m_pTarget || !m_pData)
        return;

    // TODO =====================================================
    // TODO : 여기 카메라 진도 나가고 바꾸기
    // TODO =====================================================
    // UI_GLOBAL gUI = SYS_RENDER.Get_UI_Global();

    // 일단 임시로
    const _matrix matWorld = Math::Load(m_pData->matWorld);

    const _vector vEye = XMVectorSet(0.f, 5.f, -5.f, 0.f);
    const _vector vAt = XMVectorSet(0.f, 0.f, 0.f, 0.f);
    const _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    const _matrix matView = XMMatrixLookAtLH(vEye, vAt, vUp);
    const _matrix matProj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 16.0f / 9.0f, 0.01f, 1000.0f);

    _float view[16];
    _float proj[16];
    _float world[16];

    auto MatToFloat16 = [this](const _matrix& mat, _float(&outFloat)[16])->void
        {
            _float4x4 tmpMat;
            Math::Store(tmpMat, mat);

            for (size_t i = 0; i < 4; ++i)
                for (size_t j = 0; j < 4; ++j)
                    outFloat[i * 4 + j] = tmpMat.m[i][j];
        };

    MatToFloat16(matWorld, world);
    MatToFloat16(matView, view);
    MatToFloat16(matProj, proj);

    m_pGizmo->Render(view, proj, world, vpPos, vpSize);

    if (ImGuizmo::IsUsing())
    {
        CGizmo::Apply_World_To_TransformData(world, *m_pData);
    }
}

void CScenePanel::Set_Target(Engine::CGameObject* pObj)
{
    if (m_pTarget == pObj)
        return;

    m_pTarget = pObj;
    m_pData = nullptr;
    if (!m_pTarget)
        return;

    Engine::CTransform tr = m_pTarget->Get_Component<Engine::CTransform>(Engine::COMPONENT_TYPE::TRANSFORM);
    m_pData = tr._Data();
}

void CScenePanel::Ensure_RenderTarget(_uint w, _uint h)
{
    if (m_iViewW == w && m_iViewH == h && m_pSceneSRV)
        return;

    m_iViewW = w;
    m_iViewH = h;

    SYS_CORE.Ready_SceneRenderTarget(w, h);
    SYS_CORE.Share_SceneSRV(&m_pSceneSRV); // 이제 nullptr 안 뜨게 됨
}

void CScenePanel::Render_Scene(_uint w, _uint h)
{
    // 여기서 "텍스처 없는 VIBuffer" 한 방
    // 1) 간단 셰이더 바인드(상수색 PS)
    // 2) 카메라 상수 버퍼 세팅(임시 고정 카메라라도)
    // 3) VIBuffer Render

    // 예시(네 인터페이스로 교체):
    // SYS_RENDERER.Draw_TestTriangle();
    // or m_pTestVIBuffer->Render();

    if (m_bShowGrid)
    {
        // SYS_DEBUGDRAW.DrawGrid();
    }
    if (m_bShowAxis)
    {
        // SYS_DEBUGDRAW.DrawAxis();
    }

}

std::unique_ptr<CScenePanel> CScenePanel::Create(const std::string& strPanelName, CHierarchyPanel* pHierarchy)
{
    auto p = std::make_unique<CScenePanel>(strPanelName);
    if (FAILED(p->Initialize(pHierarchy)))
        return nullptr;
    return p;
}

NS_END
