#include "Gizmo.h"

#include "Editor_Util.h"
#include "ImGuizmo.h"
#include "Transform.h"
#include "RectTransform.h"

using namespace DirectX;

NS_BEGIN(Editor)

CGizmo::CGizmo()
{
}

CGizmo::~CGizmo()
{
}

static ImGuizmo::OPERATION To_Op(GIZMO_MODE m)
{
    switch (m)
    {
    case GIZMO_MODE::TRANSLATE: return ImGuizmo::TRANSLATE;
    case GIZMO_MODE::ROTATE:    return ImGuizmo::ROTATE;
    case GIZMO_MODE::SCALE:     return ImGuizmo::SCALE;
    default:                    return ImGuizmo::TRANSLATE;
    }
}

static ImGuizmo::OPERATION To_UI_Op(GIZMO_MODE m)
{
    switch (m)
    {
    case GIZMO_MODE::TRANSLATE: return ImGuizmo::TRANSLATE; 
    case GIZMO_MODE::ROTATE:    return ImGuizmo::ROTATE_Z;
    case GIZMO_MODE::SCALE:     return ImGuizmo::SCALE_X | ImGuizmo::SCALE_Y;
    default:                    return ImGuizmo::TRANSLATE;
    }
}


static ImGuizmo::MODE To_Mode(GIZMO_SPACE s)
{
    switch (s)
    {
    case GIZMO_SPACE::LOCAL:  return ImGuizmo::LOCAL;
    case GIZMO_SPACE::WORLD:  return ImGuizmo::WORLD;
    default:                  return ImGuizmo::LOCAL;
    }
}



void CGizmo::Set_Mode(GIZMO_MODE eMode)
{
    m_Mode = eMode;
}

GIZMO_MODE CGizmo::Get_Mode() const
{
    return m_Mode;
}

GIZMO_SPACE CGizmo::Get_Space() const
{
    return m_Space;
}

void CGizmo::Set_Space(GIZMO_SPACE eSpace)
{
    m_Space = eSpace;
}

void CGizmo::Render(
    const float* view,
    const float* proj,
    float* world,
    const ImVec2& viewportPos,
    const ImVec2& viewportSize
)
{
    if (!view || !proj || !world)
        return;

    ImGuizmo::BeginFrame();
    ImGuizmo::SetOrthographic(false);

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewportSize.x, viewportSize.y);

    const ImGuizmo::OPERATION op = To_Op(m_Mode);
    const ImGuizmo::MODE mode = To_Mode(m_Space);

    ImGuizmo::Manipulate(
        view,
        proj,
        op,
        mode,
        world,
        nullptr,
        nullptr
    );

    if (ImGuizmo::IsUsing() || ImGuizmo::IsOver())
    {
        //Transpose16(world, world);
    }
}

void CGizmo::Apply_World_To_TransformData(const float* world16, tagTransformData& td)
{
    XMFLOAT4X4 m;
    memcpy(&m, world16, sizeof(float) * 16);

    XMMATRIX M = XMLoadFloat4x4(&m);

    XMVECTOR S, R, T;
    if (!XMMatrixDecompose(&S, &R, &T, M))
        return;

    XMFLOAT3 s; XMStoreFloat3(&s, S);
    XMFLOAT4 r; XMStoreFloat4(&r, R);
    XMFLOAT3 t; XMStoreFloat3(&t, T);

    td.vScale = { s.x, s.y, s.z };
    td.vRotationQuat = { r.x, r.y, r.z, r.w };
    td.vPosition = { t.x, t.y, t.z };

    td.matWorld = m;
    td.bDirty = true;
}

void CGizmo::Render_ViewAxis(
    float* view,
    const ImVec2& viewportPos,
    const ImVec2& viewportSize,
    float gizmoSize,
    float padding,
    float distance
)
{
    if (!view)
        return ;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (!window || window->SkipItems)
        return ;

    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewportSize.x, viewportSize.y);

    const ImVec2 gizmoPos(
        viewportPos.x + viewportSize.x - gizmoSize - padding,
        viewportPos.y + padding
    );

    const ImU32 bgColor = IM_COL32(0, 0, 0, 0);

    // 이 호출이 "카메라 축 기즈모" 자체입니다.
    ImGuizmo::ViewManipulate(
        view,
        distance,
        gizmoPos,
        ImVec2(gizmoSize, gizmoSize),
        bgColor
    );
}


/* TODO UI 기즈모 보류 : 전치 시키면 기즈모 안 보이고 안 시키면 Nan 이슈 아 짜증나 미치겟네 */
//void CGizmo::Render_UI(
//    const float* view,
//    const float* proj,
//    float* world,
//    const ImVec2& viewportPos,
//    const ImVec2& viewportSize
//)
//{
//    if (!view || !proj || !world) return;
//
//    ImGuizmo::BeginFrame();
//    ImGuizmo::SetOrthographic(true);
//    ImGuizmo::SetDrawlist();
//    ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewportSize.x, viewportSize.y);
//
//    const ImGuizmo::OPERATION op = To_UI_Op(m_Mode);
//    const ImGuizmo::MODE mode = ImGuizmo::LOCAL;
//
//    ImGuizmo::Manipulate(
//        view,
//        proj,
//        op,
//        mode,
//        world,
//        nullptr,
//        nullptr
//    );
//}
//
//void CGizmo::Apply_World_To_RectTransformData(const float* world16, tagRectTransformData& td, const ImVec2& viewportSize)
//{
//    float worldRM[16];
//    Editor_Util::Transpose16(world16, worldRM);
//
//    XMFLOAT4X4 m;
//    memcpy(&m, worldRM, sizeof(float) * 16);
//
//    XMMATRIX M = XMLoadFloat4x4(&m);
//
//    XMVECTOR S, R, T;
//    if (!XMMatrixDecompose(&S, &R, &T, M))
//        return;
//
//    XMFLOAT3 s; XMStoreFloat3(&s, S);
//    XMFLOAT3 t; XMStoreFloat3(&t, T);
//
//    td.vSizePx = { fabsf(s.x), fabsf(s.y) };
//    td.vPosPx = { t.x, t.y };
//
//    td.matWorld = m;
//    td.bDirty = true;
//}

std::unique_ptr<CGizmo> CGizmo::Create()
{
    return make_unique<CGizmo>();
}

NS_END
