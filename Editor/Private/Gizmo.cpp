#include "Gizmo.h"

#include "ImGuizmo.h"
#include "Transform.h"

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

static ImGuizmo::MODE To_Mode(GIZMO_SPACE s)
{
    switch (s)
    {
    case GIZMO_SPACE::LOCAL:  return ImGuizmo::LOCAL;
    case GIZMO_SPACE::WORLD:  return ImGuizmo::WORLD;
    default:                  return ImGuizmo::LOCAL;
    }
}

static void Transpose16(const float* src16, float* dst16)
{
    float t[16];
    memcpy(t, src16, sizeof(float) * 16);

    dst16[0] = t[0];  dst16[1] = t[4];  dst16[2] = t[8];  dst16[3] = t[12];
    dst16[4] = t[1];  dst16[5] = t[5];  dst16[6] = t[9];  dst16[7] = t[13];
    dst16[8] = t[2];  dst16[9] = t[6];  dst16[10] = t[10]; dst16[11] = t[14];
    dst16[12] = t[3];  dst16[13] = t[7];  dst16[14] = t[11]; dst16[15] = t[15];
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

    // SceneView 위에 그리기 위해 rect 설정
    ImGuizmo::BeginFrame();
    ImGuizmo::SetOrthographic(false);

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewportSize.x, viewportSize.y);

    // ImGuizmo가 column-major로 취급하는 경우가 많아서 transpose로 안전하게
    const ImGuizmo::OPERATION op = To_Op(m_Mode);
    const ImGuizmo::MODE mode = To_Mode(m_Space);

    // (선택) 스냅
    // float snap[3] = { 1.f, 1.f, 1.f }; // move
    // float snapAngle = 15.f;           // rotate degree
    // bool useSnap = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);

    ImGuizmo::Manipulate(
        view,
        proj,
        op,
        mode,
        world,
        nullptr,
        nullptr // useSnap ? (op==ImGuizmo::ROTATE ? &snapAngle : snap) : nullptr
    );

    if (ImGuizmo::IsUsing() || ImGuizmo::IsOver())
    {
        // 다시 DX row-major world로 되돌려서 out
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

std::unique_ptr<CGizmo> CGizmo::Create()
{
    return make_unique<CGizmo>();
}

NS_END
