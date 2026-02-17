#include "ScenePanel.h"
#include "Core_System.h"

NS_BEGIN(Editor)

CScenePanel::CScenePanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName)
{
}

CScenePanel::~CScenePanel() = default;

HRESULT CScenePanel::Initialize()
{
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
        return;
    }

    Draw_Toolbar();
    Draw_Viewport();

    ImGui::End();
}

void CScenePanel::Draw_Toolbar()
{
    if (ImGui::Button("Grid"))  m_bShowGrid = !m_bShowGrid;
    ImGui::SameLine();
    if (ImGui::Button("Axis"))  m_bShowAxis = !m_bShowAxis;

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
    ImGui::Image((ImTextureID)m_pSceneSRV, avail);
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

std::unique_ptr<CScenePanel> CScenePanel::Create(const std::string& strPanelName)
{
    auto p = std::make_unique<CScenePanel>(strPanelName);
    if (FAILED(p->Initialize()))
        return nullptr;
    return p;
}

NS_END
