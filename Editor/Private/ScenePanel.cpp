#include "ScenePanel.h"
#include "Core_System.h"
#include "HierarchyPanel.h"
#include "Gizmo.h"

#include "Transform.h"
#include "CRender_System.h"

#include "GameObject.h"
#include "CRender_System.h"
#include "Render_Context.h"
#include "Editor_System.h"

NS_BEGIN(Editor)
    CScenePanel::CScenePanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName)
{
}

CScenePanel::~CScenePanel() = default;

HRESULT CScenePanel::Initialize(CHierarchyPanel* pHierarchy)
{
    pHierarchy->m_OnPrimarySelectionChanged.Add_Listener(&CScenePanel::Set_Target, this);

    SYS_EDITOR.Update_SceneView_State((_float)m_FIXEDW, (_float)m_FIXEDH);

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

    Draw_Viewport();
    Draw_Toolbar();

    ImGui::End();
}

void CScenePanel::Draw_Toolbar()
{
    // Scene 패널(현재 윈도우) 위에 고정 오버레이 위치 잡기
    ImVec2 winPos = ImGui::GetWindowPos();       // Scene 패널 좌상단 (screen space)
    ImVec2 winPad = ImGui::GetWindowContentRegionMin(); // content 시작 오프셋
    ImVec2 pos = ImVec2(winPos.x + winPad.x + 10.0f, winPos.y + winPad.y + 10.0f);

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    // 오버레이는 별도 윈도우로 띄워야 "겹침"이 됩니다.
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);

    // 배경을 원하면 0.35f 정도, 완전 투명은 0.0f
    ImGui::SetNextWindowBgAlpha(0.35f);

    // 중요한 점: 이름을 유니크하게(같은 이름이면 합쳐질 수 있음)
    if (ImGui::Begin("##SceneToolbarOverlay", nullptr, window_flags))
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
    }
    ImGui::End();
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

    Ensure_RenderTarget();

    // -----------------------------------------------------------------
    // 16:9(=m_FIXEDW:m_FIXEDH) 유지해서 패널 안에 들어갈 최대 사각형 계산
    // -----------------------------------------------------------------
    const float rtW = (float)m_FIXEDW;
    const float rtH = (float)m_FIXEDH;
    const float rtAspect = rtW / rtH;

    const float panelW = avail.x;
    const float panelH = avail.y;
    const float panelAspect = (panelH > 0.f) ? (panelW / panelH) : rtAspect;

    ImVec2 drawSize{};
    ImVec2 offset{};

    if (panelAspect > rtAspect)
    {
        // 패널이 더 넓음 -> 높이에 맞추고 좌우 공백
        drawSize.y = panelH;
        drawSize.x = panelH * rtAspect;
        offset.x = (panelW - drawSize.x) * 0.5f;
        offset.y = 0.f;
    }
    else
    {
        // 패널이 더 높음 -> 너비에 맞추고 상하 공백
        drawSize.x = panelW;
        drawSize.y = panelW / rtAspect;
        offset.x = 0.f;
        offset.y = (panelH - drawSize.y) * 0.5f;
    }

    // 이미지 출력(레터박스)
    ImVec2 cursor = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(cursor.x + offset.x, cursor.y + offset.y));

    // UV 뒤집혀 보이면 아래 두 줄을 바꿔서 사용하세요.
    ImVec2 uv0 = ImVec2(0.f, 0.f);
    ImVec2 uv1 = ImVec2(1.f, 1.f);
    // ImVec2 uv0 = ImVec2(0.f, 1.f);
    // ImVec2 uv1 = ImVec2(1.f, 0.f);

    ImGui::Image((ImTextureID)m_pSceneSRV, drawSize, uv0, uv1);

    // 뷰포트 rect 계산 (※ 이제 avail이 아니라 "실제 이미지" 사각형이 기준)
    ImVec2 vpPos = ImGui::GetItemRectMin();
    ImVec2 vpSize = ImGui::GetItemRectSize();

    const _matrix matView =  Engine::Math::Load(SYS_RENDER.Contexts()->Get_View());
    const _matrix matProj = Engine::Math::Load(SYS_RENDER.Contexts()->Get_Proj());

    /* ----------------------- 마우스 피킹 ----------------------- */
    ImVec2 mouse = ImGui::GetMousePos();
    const bool inside =
        (mouse.x >= vpPos.x) && (mouse.y >= vpPos.y) &&
        (mouse.x < vpPos.x + vpSize.x) && (mouse.y < vpPos.y + vpSize.y);

    if (inside && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        float u = (mouse.x - vpPos.x) / vpSize.x;
        float v = (mouse.y - vpPos.y) / vpSize.y;

        if (u < 0.f) u = 0.f; if (u > 0.999999f) u = 0.999999f;
        if (v < 0.f) v = 0.f; if (v > 0.999999f) v = 0.999999f;

        const _uint px = (_uint)(u * (float)m_FIXEDW);
        const _uint py = (_uint)(v * (float)m_FIXEDH);

        SYS_EDITOR.Pick_SceneView(px, py, m_FIXEDW, m_FIXEDH);
    }

    if (!m_pTarget || !m_pData)
        return;

    /* ----------------------- 기즈모  ----------------------- */
    const _matrix matWorld = Math::Load(m_pData->matWorld);

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

void CScenePanel::Ensure_RenderTarget()
{
    /* 이미 공유되어 있으면 재호출 불필요 */
    if (m_pSceneSRV)
        return;

    SYS_CORE.Ready_SceneRenderTarget(m_FIXEDW, m_FIXEDH);
    SYS_CORE.Share_SceneSRV(&m_pSceneSRV);
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
