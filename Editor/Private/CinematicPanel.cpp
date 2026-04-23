#include "CinematicPanel.h"

#include "imgui.h"

#include "GameObject.h"
#include "Camera.h"
#include "Transform.h"

#include "CinematicIO.h"
#include "CinematicSystem.h"
#include "HierarchyPanel.h"

#include "Editor_System.h"

#include <algorithm>
#include <cmath>

using namespace Editor;
using namespace Engine;

namespace
{
    _float3 Quaternion_To_EulerDegree(const _float4& q)
    {
        const float x = q.x;
        const float y = q.y;
        const float z = q.z;
        const float w = q.w;

        const float sinr_cosp = 2.f * (w * x + y * z);
        const float cosr_cosp = 1.f - 2.f * (x * x + y * y);
        const float roll = atan2f(sinr_cosp, cosr_cosp);

        const float sinp = 2.f * (w * y - z * x);
        float pitch = 0.f;
        if (fabsf(sinp) >= 1.f)
            pitch = copysignf(DirectX::XM_PIDIV2, sinp);
        else
            pitch = asinf(sinp);

        const float siny_cosp = 2.f * (w * z + x * y);
        const float cosy_cosp = 1.f - 2.f * (y * y + z * z);
        const float yaw = atan2f(siny_cosp, cosy_cosp);

        return _float3(
            DirectX::XMConvertToDegrees(pitch),
            DirectX::XMConvertToDegrees(yaw),
            DirectX::XMConvertToDegrees(roll)
        );
    }

    _float4 EulerDegree_To_Quaternion(const _float3& vEulerDegree)
    {
        const _float pitch = DirectX::XMConvertToRadians(vEulerDegree.x);
        const _float yaw = DirectX::XMConvertToRadians(vEulerDegree.y);
        const _float roll = DirectX::XMConvertToRadians(vEulerDegree.z);

        DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);

        _float4 vQuat{};
        DirectX::XMStoreFloat4(&vQuat, q);
        return vQuat;
    }

    const char* Ease_To_String(CINEMATIC_EASE eEase)
    {
        switch (eEase)
        {
        case CINEMATIC_EASE::LINEAR:      return "LINEAR";
        case CINEMATIC_EASE::EASE_IN:     return "EASE_IN";
        case CINEMATIC_EASE::EASE_OUT:    return "EASE_OUT";
        case CINEMATIC_EASE::EASE_IN_OUT: return "EASE_IN_OUT";
        default:                          return "UNKNOWN";
        }
    }

    const char* ShotType_To_String(CINEMATIC_SHOT_TYPE eType)
    {
        switch (eType)
        {
        case CINEMATIC_SHOT_TYPE::CUT:   return "CUT";
        case CINEMATIC_SHOT_TYPE::BLEND: return "BLEND";
        default:                         return "UNKNOWN";
        }
    }

    const char* EventType_To_String(CINEMATIC_EVENT_TYPE eType)
    {
        switch (eType)
        {
        case CINEMATIC_EVENT_TYPE::NONE:   return "NONE";
        case CINEMATIC_EVENT_TYPE::CUSTOM: return "CUSTOM";
        case CINEMATIC_EVENT_TYPE::START:  return "START";
        case CINEMATIC_EVENT_TYPE::END:    return "END";
        default:                           return "UNKNOWN";
        }
    }
}

CCinematicPanel::CCinematicPanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName)
{
}

HRESULT CCinematicPanel::Initialize(CHierarchyPanel* pPanel)
{
    pPanel->m_OnPrimarySelectionChanged.Add_Listener(&CCinematicPanel::Set_Target, this);

    strcpy_s(m_tClip.szName, "new_clip");
    m_tClip.fDuration = 5.f;

    return S_OK;
}

std::unique_ptr<CCinematicPanel> CCinematicPanel::Create(const std::string& strPanelName, CHierarchyPanel* pHierarchy)
{
    auto pPanel = std::make_unique<CCinematicPanel>(strPanelName);
    pPanel->Initialize(pHierarchy);
    return pPanel;
}

void CCinematicPanel::Set_Target(CGameObject* pObj)
{
    m_pTarget = pObj;
}

void CCinematicPanel::Render()
{
    if (!m_bOpen)
        return;

    if (!ImGui::Begin(m_strPanelName.c_str(), (bool*)&m_bOpen))
    {
        ImGui::End();
        return;
    }

    Draw_Toolbar();
    ImGui::Separator();
    Draw_Workspace();

    ImGui::End();
}

void CCinematicPanel::Draw_Toolbar()
{
    ImGui::PushItemWidth(180.f);
    ImGui::InputText("Clip", m_tClip.szName, 64);
    ImGui::PopItemWidth();

    ImGui::SameLine();

    if (ImGui::Button("New"))
    {
        m_tClip = {};
        strcpy_s(m_tClip.szName, "new_clip");
        m_tClip.fDuration = 5.f;
        m_fPreviewTime = 0.f;
        m_fTimelineOffsetTime = 0.f;
        Clear_Selection();
        m_iCachedCameraKeyEulerIndex = -1;
    }

    ImGui::SameLine();
    if (ImGui::Button("Load"))
    {
        if (SYS_CINEMATIC.Load(m_tClip.szName))
        {
            SYS_CINEMATIC.Set_CurClip(m_tClip.szName);
            m_tClip = SYS_CINEMATIC.Get_CurClip();
            m_fPreviewTime = 0.f;
            m_fTimelineOffsetTime = 0.f;
            Clear_Selection();
            m_iCachedCameraKeyEulerIndex = -1;
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
        Sort_All_Keys();
        Clamp_All_Key_Time();
        CCinematicIO::Save(m_tClip.szName, m_tClip);
    }

    ImGui::SameLine();
    if (ImGui::Button("Delete Selected"))
    {
        Remove_Selected();
    }

    ImGui::SameLine();
    if (ImGui::Button("Play"))
    {
        if (m_pTarget != nullptr)
        {
            CCamera cam = m_pTarget->Get_Component<CCamera>();
            if (cam.Is_Valid())
            {
                Sort_All_Keys();
                Clamp_All_Key_Time();
                SYS_CINEMATIC.Set_TestClip(m_tClip);
                SYS_CINEMATIC.Play(cam);
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
        SYS_CINEMATIC.Stop();
    }

    ImGui::SameLine();
    ImGui::Checkbox("AutoPreview", &m_bAutoPreview);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.f);
    ImGui::InputFloat("Duration", &m_tClip.fDuration);
    if (m_tClip.fDuration < 0.1f)
        m_tClip.fDuration = 0.1f;

    if (m_pTarget)
        ImGui::Text("Target : %s", m_pTarget->Get_Label().data());
    else
        ImGui::Text("Target : None");
}

void CCinematicPanel::Draw_Workspace()
{
    const float fAvailWidth = ImGui::GetContentRegionAvail().x;
    const float fAvailHeight = ImGui::GetContentRegionAvail().y;

    const float fSafeAvailWidth = std::fmaxf(fAvailWidth, 1.f);
    const float fSafeAvailHeight = std::fmaxf(fAvailHeight, 1.f);

    const float fMaxLeftWidth = std::fmaxf(280.f, fSafeAvailWidth * 0.55f);
    const float fLeftWidth = std::clamp(m_fInspectorWidth, 280.f, fMaxLeftWidth);

    ImGui::BeginChild("InspectorPanel", ImVec2(fLeftWidth, fSafeAvailHeight), true);
    Draw_InspectorPanel();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("TimelinePanel", ImVec2(0.f, fSafeAvailHeight), true);
    Draw_TimelinePanel();
    ImGui::EndChild();
}

void CCinematicPanel::Draw_InspectorPanel()
{
    ImGui::Text("Selected Key");
    ImGui::Separator();

    if (m_iSelectedCameraKey >= 0)
    {
        Draw_CameraKey_Inspector();
        return;
    }

    if (m_iSelectedShotKey >= 0)
    {
        Draw_ShotKey_Inspector();
        return;
    }

    if (m_iSelectedEventKey >= 0)
    {
        Draw_EventKey_Inspector();
        return;
    }

    if (m_iSelectedShakeKey >= 0)
    {
        Draw_ShakeKey_Inspector();
        return;
    }

    ImGui::TextDisabled("Select Key in the timeline.");
    ImGui::Spacing();
    ImGui::TextDisabled("Quick Add");
    if (ImGui::Button("+ CameraKey", ImVec2(-1.f, 0.f)))
        Add_CameraKey_From_Selected();
    if (ImGui::Button("+ ShotKey", ImVec2(-1.f, 0.f)))
        Add_ShotKey();
    if (ImGui::Button("+ EventKey", ImVec2(-1.f, 0.f)))
        Add_EventKey();
    if (ImGui::Button("+ ShakeKey", ImVec2(-1.f, 0.f)))
        Add_ShakeKey();
}

void CCinematicPanel::Draw_TimelinePanel()
{
    Draw_TimelineTopBar();
    ImGui::Separator();

    ImVec2 vCanvasSize = ImGui::GetContentRegionAvail();
    if (vCanvasSize.y < 220.f)
        vCanvasSize.y = 220.f;

    Draw_TimelineCanvas(vCanvasSize);
}

void CCinematicPanel::Draw_TimelineTopBar()
{
    if (ImGui::Button("+ Cam"))
        Add_CameraKey_From_Selected();
    ImGui::SameLine();
    if (ImGui::Button("+ Shot"))
        Add_ShotKey();
    ImGui::SameLine();
    if (ImGui::Button("+ Event"))
        Add_EventKey();
    ImGui::SameLine();
    if (ImGui::Button("+ Shake"))
        Add_ShakeKey();

    ImGui::SameLine();
    ImGui::SetNextItemWidth(140.f);
    ImGui::SliderFloat("Zoom", &m_fTimelineZoom, 0.35f, 4.0f, "%.2f");

    ImGui::SameLine();
    if (ImGui::Button("Focus Preview"))
        Focus_Time(m_fPreviewTime);

    ImGui::SameLine();
    if (ImGui::Button("Sort All"))
        Sort_All_Keys();

    ImGui::SameLine();
    ImGui::Text("Preview %.2f / %.2f", m_fPreviewTime, m_tClip.fDuration);
}

void CCinematicPanel::Draw_CameraKey_Inspector()
{
    if (m_iSelectedCameraKey < 0 || m_iSelectedCameraKey >= (int)m_tClip.vecCameraKeys.size())
        return;

    auto& t = m_tClip.vecCameraKeys[m_iSelectedCameraKey];

    ImGui::Text("Camera Key #%d", m_iSelectedCameraKey);
    ImGui::Separator();

    ImGui::SliderFloat("Time", &t.fTime, 0.f, m_tClip.fDuration, "%.2f");

    ImGui::InputFloat3("Position", &t.vPosition.x);
    ImGui::InputFloat("Fovy", &t.fFovy);

    /* 선택된 CameraKey가 바뀐 경우에만 1회 동기화 */
    if (m_iCachedCameraKeyEulerIndex != m_iSelectedCameraKey)
    {
        m_vCachedCameraKeyEuler = Quaternion_To_EulerDegree(t.vRotationQuat);
        m_iCachedCameraKeyEulerIndex = m_iSelectedCameraKey;
    }

    _float vEuler[3] =
    {
        m_vCachedCameraKeyEuler.x,
        m_vCachedCameraKeyEuler.y,
        m_vCachedCameraKeyEuler.z
    };

    if (ImGui::InputFloat3("Rotation Euler", vEuler))
    {
        m_vCachedCameraKeyEuler = { vEuler[0], vEuler[1], vEuler[2] };
        t.vRotationQuat = EulerDegree_To_Quaternion(m_vCachedCameraKeyEuler);
    }

    if (ImGui::Button("Current", ImVec2(-1.f, 0.f)))
    {
        const _float3 vPos = SYS_EDITOR.Get_Position();
        const _float3 vRot = SYS_EDITOR.Get_RotationEuler();

        t.vPosition = vPos;
        m_vCachedCameraKeyEuler = vRot;
        t.vRotationQuat = EulerDegree_To_Quaternion(m_vCachedCameraKeyEuler);
    }

    int iEase = (int)t.eEase;
    const char* easeItems[] = { "LINEAR", "EASE_IN", "EASE_OUT", "EASE_IN_OUT" };
    if (ImGui::Combo("Ease", &iEase, easeItems, IM_ARRAYSIZE(easeItems)))
        t.eEase = (CINEMATIC_EASE)std::clamp(iEase, 0, 3);

    ImGui::Spacing();

    if (ImGui::Button("Capture From Target", ImVec2(-1.f, 0.f)))
        Capture_Selected_CameraKey_From_Target();

    if (ImGui::Button("Apply To Target", ImVec2(-1.f, 0.f)))
        Apply_Selected_CameraKey_To_Target();

    if (ImGui::Button("Jump Preview Here", ImVec2(-1.f, 0.f)))
    {
        m_fPreviewTime = t.fTime;
        Focus_Time(t.fTime);
        Preview_CurrentTime();
    }

    Clamp_All_Key_Time();
}

void CCinematicPanel::Draw_ShotKey_Inspector()
{
    if (m_iSelectedShotKey < 0 || m_iSelectedShotKey >= (int)m_tClip.vecShotKeys.size())
        return;

    auto& t = m_tClip.vecShotKeys[m_iSelectedShotKey];

    ImGui::Text("Shot Key #%d", m_iSelectedShotKey);
    ImGui::Separator();

    ImGui::SliderFloat("Shot Time", &t.fTime, 0.f, m_tClip.fDuration, "%.2f");

    int iType = (int)t.eType;
    const char* typeItems[] = { "CUT", "BLEND" };
    if (ImGui::Combo("Type", &iType, typeItems, IM_ARRAYSIZE(typeItems)))
        t.eType = (CINEMATIC_SHOT_TYPE)std::clamp(iType, 0, 1);

    if (!m_tClip.vecCameraKeys.empty())
    {
        std::vector<std::string> vecLabels;
        std::vector<const char*> vecItems;
        vecLabels.reserve(m_tClip.vecCameraKeys.size());
        vecItems.reserve(m_tClip.vecCameraKeys.size());

        for (size_t i = 0; i < m_tClip.vecCameraKeys.size(); ++i)
        {
            char szBuf[64];
            sprintf_s(szBuf, "Cam %zu (%.2f)", i, m_tClip.vecCameraKeys[i].fTime);
            vecLabels.push_back(szBuf);
        }

        for (auto& str : vecLabels)
            vecItems.push_back(str.c_str());

        int iCameraKeyIndex = (int)t.iCameraKeyIndex;
        iCameraKeyIndex = std::clamp(iCameraKeyIndex, 0, (int)m_tClip.vecCameraKeys.size() - 1);

        if (ImGui::Combo("Camera Key", &iCameraKeyIndex, vecItems.data(), (int)vecItems.size()))
            t.iCameraKeyIndex = (uint32_t)iCameraKeyIndex;
    }
    else
    {
        ImGui::TextDisabled("CameraKey가 없습니다.");
        t.iCameraKeyIndex = 0;
    }

    if (t.eType == CINEMATIC_SHOT_TYPE::BLEND)
    {
        ImGui::SliderFloat("Blend Duration", &t.fBlendDuration, 0.f, 5.f, "%.2f");
    }

    ImGui::Separator();
    ImGui::Checkbox("Use Orbit", &t.bUseOrbit);

    if (t.bUseOrbit)
    {
        ImGui::InputFloat3("Orbit Center", &t.vOrbitCenter.x);
        ImGui::InputFloat("Orbit Radius", &t.fOrbitRadius);
        ImGui::InputFloat("Orbit Start Height", &t.fOrbitStartHeightOffset);
        ImGui::InputFloat("Orbit End Height", &t.fOrbitEndHeightOffset);
        ImGui::InputFloat("Orbit Start Angle", &t.fOrbitStartAngleDeg);
        ImGui::InputFloat("Orbit Sweep Angle", &t.fOrbitSweepAngleDeg);

        if (ImGui::Button("Current Orbit Center", ImVec2(-1.f, 0.f)))
        {
            t.vOrbitCenter = SYS_EDITOR.Get_Position();
        }

        if (ImGui::Button("Bake Orbit From CameraKey", ImVec2(-1.f, 0.f)))
        {
            if (t.iCameraKeyIndex < m_tClip.vecCameraKeys.size())
            {
                const auto& tCam = m_tClip.vecCameraKeys[t.iCameraKeyIndex];

                const _float dx = tCam.vPosition.x - t.vOrbitCenter.x;
                const _float dz = tCam.vPosition.z - t.vOrbitCenter.z;

                t.fOrbitRadius = sqrtf(dx * dx + dz * dz);
                t.fOrbitStartHeightOffset = tCam.vPosition.y - t.vOrbitCenter.y;
                t.fOrbitEndHeightOffset = t.fOrbitStartHeightOffset;
                t.fOrbitStartAngleDeg = DirectX::XMConvertToDegrees(atan2f(dz, dx));
            }
        }
    }

    ImGui::Separator();
    ImGui::Checkbox("Use LookAt", &t.bUseLookAt);

    if (t.bUseLookAt)
    {
        ImGui::InputFloat3("LookAt Position", &t.vLookAtPosition.x);
        ImGui::SliderFloat("LookAt Blend", &t.fLookAtBlendRatio, 0.f, 1.f, "%.2f");

        if (ImGui::Button("Current LookAt", ImVec2(-1.f, 0.f)))
        {
            t.vLookAtPosition = SYS_EDITOR.Get_Position();
        }
    }

    if (ImGui::Button("Jump Preview Here", ImVec2(-1.f, 0.f)))
    {
        m_fPreviewTime = t.fTime;
        Focus_Time(t.fTime);
        Preview_CurrentTime();
    }

    Clamp_All_Key_Time();
}

void CCinematicPanel::Draw_EventKey_Inspector()
{
    if (m_iSelectedEventKey < 0 || m_iSelectedEventKey >= (int)m_tClip.vecEventKeys.size())
        return;

    auto& t = m_tClip.vecEventKeys[m_iSelectedEventKey];

    ImGui::Text("Event Key #%d", m_iSelectedEventKey);
    ImGui::Separator();

    ImGui::SliderFloat("Event Time", &t.fTime, 0.f, m_tClip.fDuration, "%.2f");

    int iType = (int)t.eType;
    const char* typeItems[] = { "NONE", "CUSTOM", "START", "END" };
    if (ImGui::Combo("Event Type", &iType, typeItems, IM_ARRAYSIZE(typeItems)))
        t.eType = (CINEMATIC_EVENT_TYPE)std::clamp(iType, 0, 3);

    ImGui::InputText("Event Name", t.szEventName, 64);

    if (ImGui::Button("Jump Preview Here", ImVec2(-1.f, 0.f)))
    {
        m_fPreviewTime = t.fTime;
        Focus_Time(t.fTime);
        Preview_CurrentTime();
    }

    Clamp_All_Key_Time();
}

void CCinematicPanel::Draw_ShakeKey_Inspector()
{
    if (m_iSelectedShakeKey < 0 || m_iSelectedShakeKey >= (int)m_tClip.vecShakeKeys.size())
        return;

    auto& t = m_tClip.vecShakeKeys[m_iSelectedShakeKey];

    ImGui::Text("Shake Key #%d", m_iSelectedShakeKey);
    ImGui::Separator();

    ImGui::SliderFloat("Start Time", &t.fStartTime, 0.f, m_tClip.fDuration, "%.2f");
    ImGui::SliderFloat("End Time", &t.fEndTime, 0.f, m_tClip.fDuration, "%.2f");

    if (t.fEndTime < t.fStartTime)
        t.fEndTime = t.fStartTime;

    ImGui::SliderFloat("Amplitude Pos", &t.fAmplitudePos, 0.f, 3.f, "%.2f");
    ImGui::SliderFloat("Amplitude Rot", &t.fAmplitudeRot, 0.f, 20.f, "%.2f");
    ImGui::SliderFloat("Frequency", &t.fFrequency, 0.f, 60.f, "%.2f");

    if (ImGui::Button("Jump Preview Here", ImVec2(-1.f, 0.f)))
    {
        m_fPreviewTime = t.fStartTime;
        Focus_Time(t.fStartTime);
        Preview_CurrentTime();
    }

    Clamp_All_Key_Time();
}

void CCinematicPanel::Add_CameraKey_From_Selected()
{
    if (m_pTarget == nullptr)
        return;

    CCamera cam = m_pTarget->Get_Component<CCamera>();
    CTransform tr = m_pTarget->Get_Component<CTransform>();

    if (!cam.Is_Valid() || !tr.Is_Valid())
        return;

    CINEMATIC_CAMERA_KEY t{};

    _float3 vPos{};
    DirectX::XMStoreFloat3(&vPos, tr.Get_StateXM(STATE::POSITION));

    t.fTime = m_fPreviewTime;
    t.vPosition = vPos;
    t.vRotationQuat = tr.Get_Rotation_Quaternion();
    t.fFovy = cam->fFovy;
    t.eEase = CINEMATIC_EASE::LINEAR;

    m_tClip.vecCameraKeys.push_back(t);
    Clear_Selection();
    m_iSelectedCameraKey = (int)m_tClip.vecCameraKeys.size() - 1;

    Sort_All_Keys();
    Focus_Time(t.fTime);

    m_iCachedCameraKeyEulerIndex = -1;
}

void CCinematicPanel::Add_ShotKey()
{
    CINEMATIC_SHOT_KEY t{};
    t.fTime = m_fPreviewTime;
    t.eType = CINEMATIC_SHOT_TYPE::CUT;
    t.iCameraKeyIndex = 0;
    t.fBlendDuration = 1.f;

    t.bUseLookAt = false;
    t.vLookAtPosition = { 0.f, 0.f, 0.f };
    t.fLookAtBlendRatio = 1.f;

    t.bUseOrbit = false;
    t.vOrbitCenter = { 0.f, 0.f, 0.f };
    t.fOrbitRadius = 5.f;
    t.fOrbitStartHeightOffset = 0.f;
    t.fOrbitEndHeightOffset = 0.f;
    t.fOrbitStartAngleDeg = 0.f;
    t.fOrbitSweepAngleDeg = 180.f;

    m_tClip.vecShotKeys.push_back(t);
    Clear_Selection();
    m_iSelectedShotKey = (int)m_tClip.vecShotKeys.size() - 1;

    Sort_All_Keys();
    Focus_Time(t.fTime);
}

void CCinematicPanel::Add_EventKey()
{
    CINEMATIC_EVENT_KEY t{};
    t.fTime = m_fPreviewTime;
    t.eType = CINEMATIC_EVENT_TYPE::CUSTOM;
    strcpy_s(t.szEventName, "Event");

    m_tClip.vecEventKeys.push_back(t);
    Clear_Selection();
    m_iSelectedEventKey = (int)m_tClip.vecEventKeys.size() - 1;

    Sort_All_Keys();
    Focus_Time(t.fTime);
}

void CCinematicPanel::Add_ShakeKey()
{
    CINEMATIC_SHAKE_KEY t{};
    t.fStartTime = m_fPreviewTime;
    t.fEndTime = std::fminf(m_fPreviewTime + 0.5f, m_tClip.fDuration);
    t.fAmplitudePos = 0.05f;
    t.fAmplitudeRot = 1.f;
    t.fFrequency = 16.f;

    m_tClip.vecShakeKeys.push_back(t);
    Clear_Selection();
    m_iSelectedShakeKey = (int)m_tClip.vecShakeKeys.size() - 1;

    Sort_All_Keys();
    Focus_Time(t.fStartTime);
}

void CCinematicPanel::Remove_Selected()
{
    if (m_iSelectedCameraKey >= 0 && m_iSelectedCameraKey < (int)m_tClip.vecCameraKeys.size())
    {
        m_tClip.vecCameraKeys.erase(m_tClip.vecCameraKeys.begin() + m_iSelectedCameraKey);
        m_iSelectedCameraKey = -1;
        return;
    }

    if (m_iSelectedShotKey >= 0 && m_iSelectedShotKey < (int)m_tClip.vecShotKeys.size())
    {
        m_tClip.vecShotKeys.erase(m_tClip.vecShotKeys.begin() + m_iSelectedShotKey);
        m_iSelectedShotKey = -1;
        return;
    }

    if (m_iSelectedEventKey >= 0 && m_iSelectedEventKey < (int)m_tClip.vecEventKeys.size())
    {
        m_tClip.vecEventKeys.erase(m_tClip.vecEventKeys.begin() + m_iSelectedEventKey);
        m_iSelectedEventKey = -1;
        return;
    }

    if (m_iSelectedShakeKey >= 0 && m_iSelectedShakeKey < (int)m_tClip.vecShakeKeys.size())
    {
        m_tClip.vecShakeKeys.erase(m_tClip.vecShakeKeys.begin() + m_iSelectedShakeKey);
        m_iSelectedShakeKey = -1;
        return;
    }
}

void CCinematicPanel::Sort_All_Keys()
{
    std::sort(m_tClip.vecCameraKeys.begin(), m_tClip.vecCameraKeys.end(),
        [](const CINEMATIC_CAMERA_KEY& a, const CINEMATIC_CAMERA_KEY& b)
        {
            return a.fTime < b.fTime;
        });

    std::sort(m_tClip.vecShotKeys.begin(), m_tClip.vecShotKeys.end(),
        [](const CINEMATIC_SHOT_KEY& a, const CINEMATIC_SHOT_KEY& b)
        {
            return a.fTime < b.fTime;
        });

    std::sort(m_tClip.vecEventKeys.begin(), m_tClip.vecEventKeys.end(),
        [](const CINEMATIC_EVENT_KEY& a, const CINEMATIC_EVENT_KEY& b)
        {
            return a.fTime < b.fTime;
        });

    std::sort(m_tClip.vecShakeKeys.begin(), m_tClip.vecShakeKeys.end(),
        [](const CINEMATIC_SHAKE_KEY& a, const CINEMATIC_SHAKE_KEY& b)
        {
            return a.fStartTime < b.fStartTime;
        });
}

void CCinematicPanel::Clear_Selection()
{
    m_iSelectedCameraKey = -1;
    m_iSelectedShotKey = -1;
    m_iSelectedEventKey = -1;
    m_iSelectedShakeKey = -1;

    m_iCachedCameraKeyEulerIndex = -1;
    m_vCachedCameraKeyEuler = { 0.f, 0.f, 0.f };
}

void CCinematicPanel::Preview_CurrentTime()
{
    if (m_pTarget == nullptr)
        return;

    CCamera cam = m_pTarget->Get_Component<CCamera>();
    if (!cam.Is_Valid())
        return;

    Sort_All_Keys();
    Clamp_All_Key_Time();

    LOG_INFO("==== Preview_CurrentTime ====");
    LOG_INFO("m_fPreviewTime = %.2f", m_fPreviewTime);

    if (m_iSelectedCameraKey >= 0 && m_iSelectedCameraKey < (int)m_tClip.vecCameraKeys.size())
    {
        const auto& tKey = m_tClip.vecCameraKeys[m_iSelectedCameraKey];
        LOG_INFO("Selected CameraKey Index = %d", m_iSelectedCameraKey);
        LOG_INFO("Selected CameraKey Pos = %.2f, %.2f, %.2f",
            tKey.vPosition.x, tKey.vPosition.y, tKey.vPosition.z);
    }
    else
    {
        LOG_INFO("Selected CameraKey Index = NONE");
    }

    SYS_CINEMATIC.Set_TestClip(m_tClip);
    SYS_CINEMATIC.Preview(cam, m_fPreviewTime);
}

void CCinematicPanel::Capture_Selected_CameraKey_From_Target()
{
    if (m_iSelectedCameraKey < 0 || m_iSelectedCameraKey >= (int)m_tClip.vecCameraKeys.size())
        return;

    if (m_pTarget == nullptr)
        return;

    CCamera cam = m_pTarget->Get_Component<CCamera>();
    CTransform tr = m_pTarget->Get_Component<CTransform>();

    if (!cam.Is_Valid() || !tr.Is_Valid())
        return;

    auto& t = m_tClip.vecCameraKeys[m_iSelectedCameraKey];

    _float3 vPos{};
    DirectX::XMStoreFloat3(&vPos, tr.Get_StateXM(STATE::POSITION));

    t.vPosition = vPos;
    t.vRotationQuat = tr.Get_Rotation_Quaternion();
    t.fFovy = cam->fFovy;

    m_iCachedCameraKeyEulerIndex = -1;
}

void CCinematicPanel::Apply_Selected_CameraKey_To_Target()
{
    if (m_iSelectedCameraKey < 0 || m_iSelectedCameraKey >= (int)m_tClip.vecCameraKeys.size())
        return;

    if (m_pTarget == nullptr)
        return;

    CCamera cam = m_pTarget->Get_Component<CCamera>();
    CTransform tr = m_pTarget->Get_Component<CTransform>();

    if (!cam.Is_Valid() || !tr.Is_Valid())
        return;

    const auto& t = m_tClip.vecCameraKeys[m_iSelectedCameraKey];

    tr.Set_Position(DirectX::XMLoadFloat3(&t.vPosition));
    tr.Set_Rotation_Quaternion(DirectX::XMLoadFloat4(&t.vRotationQuat));
    cam.Set_Fovy(t.fFovy);

    tr->bDirty = true;
    cam->dirty = true;
}

_float CCinematicPanel::Get_PixelsPerSecond() const
{
    return 140.f * m_fTimelineZoom;
}

_float CCinematicPanel::Get_VisibleDuration(_float fTrackWidth) const
{
    return std::fmaxf(0.1f, fTrackWidth / Get_PixelsPerSecond());
}

void CCinematicPanel::Focus_Time(_float fTime)
{
    const float fVisible = 3.0f;
    m_fTimelineOffsetTime = std::fmaxf(0.f, fTime - fVisible * 0.5f);
}

void CCinematicPanel::Clamp_TimelineView(_float fTrackWidth)
{
    const _float fVisibleDuration = Get_VisibleDuration(fTrackWidth);
    const _float fMaxOffset = std::fmaxf(0.f, m_tClip.fDuration - fVisibleDuration);
    m_fTimelineOffsetTime = std::clamp(m_fTimelineOffsetTime, 0.f, fMaxOffset);
}

void CCinematicPanel::Clamp_All_Key_Time()
{
    m_fPreviewTime = std::clamp(m_fPreviewTime, 0.f, m_tClip.fDuration);

    for (auto& t : m_tClip.vecCameraKeys)
        t.fTime = std::clamp(t.fTime, 0.f, m_tClip.fDuration);

    for (auto& t : m_tClip.vecShotKeys)
    {
        t.fTime = std::clamp(t.fTime, 0.f, m_tClip.fDuration);

        if (!m_tClip.vecCameraKeys.empty())
            t.iCameraKeyIndex = std::min<uint32_t>(t.iCameraKeyIndex, (uint32_t)m_tClip.vecCameraKeys.size() - 1);
        else
            t.iCameraKeyIndex = 0;

        if (t.fBlendDuration < 0.f)
            t.fBlendDuration = 0.f;

        t.fLookAtBlendRatio = std::clamp(t.fLookAtBlendRatio, 0.f, 1.f);

        if (t.fOrbitRadius < 0.01f)
            t.fOrbitRadius = 0.01f;

    }

    for (auto& t : m_tClip.vecEventKeys)
        t.fTime = std::clamp(t.fTime, 0.f, m_tClip.fDuration);

    for (auto& t : m_tClip.vecShakeKeys)
    {
        t.fStartTime = std::clamp(t.fStartTime, 0.f, m_tClip.fDuration);
        t.fEndTime = std::clamp(t.fEndTime, 0.f, m_tClip.fDuration);
        if (t.fEndTime < t.fStartTime)
            t.fEndTime = t.fStartTime;
    }
}

_float CCinematicPanel::Time_To_X(_float fTime, _float fStartX, _float fWidth) const
{
    const _float fPps = Get_PixelsPerSecond();
    return fStartX + (fTime - m_fTimelineOffsetTime) * fPps;
}

_float CCinematicPanel::X_To_Time(_float fX, _float fStartX, _float fWidth) const
{
    const _float fPps = Get_PixelsPerSecond();
    const _float fLocal = fX - fStartX;
    return std::clamp(m_fTimelineOffsetTime + fLocal / std::fmaxf(1.f, fPps), 0.f, m_tClip.fDuration);
}

void CCinematicPanel::Draw_TrackRowLabel(const char* pLabel, _float fY, ImDrawList* pDraw, const ImVec2& vMin, const ImVec2& vMax)
{
    pDraw->AddText(ImVec2(vMin.x + 10.f, fY - 8.f), IM_COL32(220, 220, 220, 255), pLabel);
}

void CCinematicPanel::Draw_TimeRuler(ImDrawList* pDraw, const ImVec2& vMin, const ImVec2& vMax, _float fTrackStartX, _float fTrackWidth)
{
    const _float fVisibleDuration = Get_VisibleDuration(fTrackWidth);

    _float fStep = 0.5f;
    if (fVisibleDuration > 10.f) fStep = 1.f;
    if (fVisibleDuration > 20.f) fStep = 2.f;
    if (fVisibleDuration > 40.f) fStep = 5.f;

    const _float fStart = floorf(m_fTimelineOffsetTime / fStep) * fStep;
    const _float fEnd = m_fTimelineOffsetTime + fVisibleDuration + fStep;

    for (_float t = fStart; t <= fEnd; t += fStep)
    {
        if (t < 0.f)
            continue;

        const _float fX = Time_To_X(t, fTrackStartX, fTrackWidth);
        if (fX < fTrackStartX || fX > fTrackStartX + fTrackWidth)
            continue;

        pDraw->AddLine(ImVec2(fX, vMin.y + 24.f), ImVec2(fX, vMax.y - 8.f), IM_COL32(70, 70, 80, 130), 1.f);

        char szBuf[32];
        sprintf_s(szBuf, "%.1f", t);
        pDraw->AddText(ImVec2(fX - 10.f, vMin.y + 4.f), IM_COL32(185, 185, 185, 255), szBuf);
    }
}

void CCinematicPanel::Handle_TimelineInput(const ImVec2& vMin, const ImVec2& vMax, _float fTrackStartX, _float fTrackWidth)
{
    ImGuiIO& io = ImGui::GetIO();

    if (!ImGui::IsItemHovered())
        return;

    if (io.MouseWheel != 0.f)
    {
        if (io.KeyCtrl)
        {
            const _float fPrevZoom = m_fTimelineZoom;
            m_fTimelineZoom = std::clamp(m_fTimelineZoom + io.MouseWheel * 0.1f, 0.35f, 4.0f);

            const _float fMouseTime = X_To_Time(io.MousePos.x, fTrackStartX, fTrackWidth);
            const _float fVisibleDuration = Get_VisibleDuration(fTrackWidth);
            m_fTimelineOffsetTime = std::clamp(fMouseTime - fVisibleDuration * 0.5f, 0.f, std::fmaxf(0.f, m_tClip.fDuration - fVisibleDuration));
        }
        else
        {
            m_fTimelineOffsetTime -= io.MouseWheel * (Get_VisibleDuration(fTrackWidth) * 0.1f);
        }
    }

    Clamp_TimelineView(fTrackWidth);

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
    {
        m_fTimelineOffsetTime -= io.MouseDelta.x / std::fmaxf(1.f, Get_PixelsPerSecond());
        Clamp_TimelineView(fTrackWidth);
    }
}

void CCinematicPanel::Draw_TimelineCanvas(const ImVec2& vCanvasSize)
{
    const _float fCanvasWidth = std::fmaxf(1.f, vCanvasSize.x);
    const _float fCanvasHeight = std::fmaxf(220.f, vCanvasSize.y);

    const ImVec2 vMin = ImGui::GetCursorScreenPos();
    const ImVec2 vMax(vMin.x + fCanvasWidth, vMin.y + fCanvasHeight);

    ImGui::InvisibleButton("##TimelineCanvas", ImVec2(fCanvasWidth, fCanvasHeight),
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);

    ImDrawList* pDraw = ImGui::GetWindowDrawList();
    pDraw->AddRectFilled(vMin, vMax, IM_COL32(28, 28, 32, 255), 6.f);
    pDraw->AddRect(vMin, vMax, IM_COL32(80, 80, 88, 255), 6.f);

    const _float fHeaderH = 28.f;
    const _float fRowH = 44.f;
    const _float fLabelW = 86.f;
    const _float fTrackStartX = vMin.x + fLabelW;
    const _float fTrackWidth = fCanvasWidth - fLabelW - 8.f;

    Clamp_TimelineView(fTrackWidth);

    const _float fCamY = vMin.y + fHeaderH + fRowH * 0.5f;
    const _float fShotY = vMin.y + fHeaderH + fRowH * 1.5f;
    const _float fEventY = vMin.y + fHeaderH + fRowH * 2.5f;
    const _float fShakeY = vMin.y + fHeaderH + fRowH * 3.5f;

    for (int i = 0; i < 4; ++i)
    {
        const _float y0 = vMin.y + fHeaderH + fRowH * i;
        const _float y1 = y0 + fRowH;
        const ImU32 col = (i % 2 == 0) ? IM_COL32(34, 34, 38, 255) : IM_COL32(40, 40, 44, 255);
        pDraw->AddRectFilled(ImVec2(vMin.x, y0), ImVec2(vMax.x, y1), col);
        pDraw->AddLine(ImVec2(vMin.x, y1), ImVec2(vMax.x, y1), IM_COL32(60, 60, 66, 255));
    }

    pDraw->AddRectFilled(ImVec2(vMin.x, vMin.y), ImVec2(vMax.x, vMin.y + fHeaderH), IM_COL32(36, 36, 42, 255), 6.f, ImDrawFlags_RoundCornersTop);
    pDraw->AddLine(ImVec2(fTrackStartX - 1.f, vMin.y), ImVec2(fTrackStartX - 1.f, vMax.y), IM_COL32(88, 88, 96, 255));

    Draw_TrackRowLabel("Camera", fCamY, pDraw, vMin, vMax);
    Draw_TrackRowLabel("Shot", fShotY, pDraw, vMin, vMax);
    Draw_TrackRowLabel("Event", fEventY, pDraw, vMin, vMax);
    Draw_TrackRowLabel("Shake", fShakeY, pDraw, vMin, vMax);

    Draw_TimeRuler(pDraw, vMin, vMax, fTrackStartX, fTrackWidth);

    pDraw->AddLine(ImVec2(fTrackStartX, fCamY), ImVec2(fTrackStartX + fTrackWidth, fCamY), IM_COL32(96, 96, 104, 255));
    pDraw->AddLine(ImVec2(fTrackStartX, fShotY), ImVec2(fTrackStartX + fTrackWidth, fShotY), IM_COL32(96, 96, 104, 255));
    pDraw->AddLine(ImVec2(fTrackStartX, fEventY), ImVec2(fTrackStartX + fTrackWidth, fEventY), IM_COL32(96, 96, 104, 255));
    pDraw->AddLine(ImVec2(fTrackStartX, fShakeY), ImVec2(fTrackStartX + fTrackWidth, fShakeY), IM_COL32(96, 96, 104, 255));

    m_iHoveredCameraKey = -1;
    m_iHoveredShotKey = -1;
    m_iHoveredEventKey = -1;
    m_iHoveredShakeKey = -1;

    const ImVec2 vMouse = ImGui::GetIO().MousePos;

    for (int i = 0; i < (int)m_tClip.vecCameraKeys.size(); ++i)
    {
        const auto& t = m_tClip.vecCameraKeys[i];
        const _float x = Time_To_X(t.fTime, fTrackStartX, fTrackWidth);

        if (x < fTrackStartX - 12.f || x > fTrackStartX + fTrackWidth + 12.f)
            continue;

        const bool bHovered = (fabsf(vMouse.x - x) <= 8.f && fabsf(vMouse.y - fCamY) <= 10.f);
        if (bHovered)
            m_iHoveredCameraKey = i;

        const ImU32 col = (m_iSelectedCameraKey == i) ? IM_COL32(255, 230, 120, 255) :
            (bHovered ? IM_COL32(150, 220, 255, 255) : IM_COL32(95, 190, 255, 255));

        pDraw->AddCircleFilled(ImVec2(x, fCamY), 7.f, col);
        pDraw->AddCircle(ImVec2(x, fCamY), 7.f, IM_COL32(20, 20, 20, 255), 0, 2.f);
    }

    for (int i = 0; i < (int)m_tClip.vecShotKeys.size(); ++i)
    {
        const auto& t = m_tClip.vecShotKeys[i];
        const _float x = Time_To_X(t.fTime, fTrackStartX, fTrackWidth);

        if (x < fTrackStartX - 12.f || x > fTrackStartX + fTrackWidth + 12.f)
            continue;

        const bool bHovered = (fabsf(vMouse.x - x) <= 8.f && fabsf(vMouse.y - fShotY) <= 10.f);
        if (bHovered)
            m_iHoveredShotKey = i;

        const ImU32 col = (m_iSelectedShotKey == i) ? IM_COL32(255, 225, 120, 255) :
            (t.eType == CINEMATIC_SHOT_TYPE::CUT ? IM_COL32(255, 120, 120, 255) : IM_COL32(255, 170, 70, 255));

        pDraw->AddTriangleFilled(
            ImVec2(x, fShotY - 8.f),
            ImVec2(x - 7.f, fShotY + 6.f),
            ImVec2(x + 7.f, fShotY + 6.f),
            col
        );

        if (t.eType == CINEMATIC_SHOT_TYPE::BLEND && t.fBlendDuration > 0.f)
        {
            const _float x1 = Time_To_X(std::fminf(t.fTime + t.fBlendDuration, m_tClip.fDuration), fTrackStartX, fTrackWidth);
            pDraw->AddLine(ImVec2(x, fShotY), ImVec2(x1, fShotY), IM_COL32(255, 190, 90, 220), 3.f);
        }
    }

    for (int i = 0; i < (int)m_tClip.vecEventKeys.size(); ++i)
    {
        const auto& t = m_tClip.vecEventKeys[i];
        const _float x = Time_To_X(t.fTime, fTrackStartX, fTrackWidth);

        if (x < fTrackStartX - 12.f || x > fTrackStartX + fTrackWidth + 12.f)
            continue;

        const bool bHovered = (fabsf(vMouse.x - x) <= 8.f && fabsf(vMouse.y - fEventY) <= 10.f);
        if (bHovered)
            m_iHoveredEventKey = i;

        const ImU32 col = (m_iSelectedEventKey == i) ? IM_COL32(255, 255, 120, 255) :
            IM_COL32(120, 235, 140, 255);

        pDraw->AddRectFilled(ImVec2(x - 6.f, fEventY - 6.f), ImVec2(x + 6.f, fEventY + 6.f), col, 2.f);
    }

    for (int i = 0; i < (int)m_tClip.vecShakeKeys.size(); ++i)
    {
        const auto& t = m_tClip.vecShakeKeys[i];
        const _float x0 = Time_To_X(t.fStartTime, fTrackStartX, fTrackWidth);
        const _float x1 = Time_To_X(t.fEndTime, fTrackStartX, fTrackWidth);

        const _float y0 = fShakeY - 10.f;
        const _float y1 = fShakeY + 10.f;

        const bool bHovered = (vMouse.x >= x0 - 3.f && vMouse.x <= x1 + 3.f && vMouse.y >= y0 - 3.f && vMouse.y <= y1 + 3.f);
        if (bHovered)
            m_iHoveredShakeKey = i;

        const ImU32 col = (m_iSelectedShakeKey == i) ? IM_COL32(255, 180, 255, 255) : IM_COL32(205, 110, 255, 190);

        pDraw->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), col, 4.f);
        pDraw->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(255, 255, 255, 100), 4.f);
        pDraw->AddLine(ImVec2(x0, y0), ImVec2(x0, y1), IM_COL32(255, 255, 255, 180), 2.f);
        pDraw->AddLine(ImVec2(x1, y0), ImVec2(x1, y1), IM_COL32(255, 255, 255, 180), 2.f);
    }

    const _float fPreviewX = Time_To_X(m_fPreviewTime, fTrackStartX, fTrackWidth);
    pDraw->AddLine(ImVec2(fPreviewX, vMin.y + 2.f), ImVec2(fPreviewX, vMax.y - 2.f), IM_COL32(255, 255, 255, 230), 2.f);
    pDraw->AddTriangleFilled(ImVec2(fPreviewX, vMin.y + 2.f), ImVec2(fPreviewX - 6.f, vMin.y + 14.f), ImVec2(fPreviewX + 6.f, vMin.y + 14.f), IM_COL32(255, 255, 255, 230));

    Handle_TimelineInput(vMin, vMax, fTrackStartX, fTrackWidth);

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        const bool bPreviewHeadHit = (fabsf(vMouse.x - fPreviewX) <= 7.f);

        if (bPreviewHeadHit)
        {
            m_eDragTarget = TIMELINE_DRAG_TARGET::PREVIEW;
            m_fDragTimeOffset = m_fPreviewTime - X_To_Time(vMouse.x, fTrackStartX, fTrackWidth);
        }
        else if (m_iHoveredCameraKey >= 0)
        {
            Clear_Selection();
            m_iSelectedCameraKey = m_iHoveredCameraKey;
            m_eDragTarget = TIMELINE_DRAG_TARGET::CAMERA;
            m_fDragTimeOffset = m_tClip.vecCameraKeys[m_iSelectedCameraKey].fTime - X_To_Time(vMouse.x, fTrackStartX, fTrackWidth);
        }
        else if (m_iHoveredShotKey >= 0)
        {
            Clear_Selection();
            m_iSelectedShotKey = m_iHoveredShotKey;
            m_eDragTarget = TIMELINE_DRAG_TARGET::SHOT;
            m_fDragTimeOffset = m_tClip.vecShotKeys[m_iSelectedShotKey].fTime - X_To_Time(vMouse.x, fTrackStartX, fTrackWidth);
        }
        else if (m_iHoveredEventKey >= 0)
        {
            Clear_Selection();
            m_iSelectedEventKey = m_iHoveredEventKey;
            m_eDragTarget = TIMELINE_DRAG_TARGET::EVENT;
            m_fDragTimeOffset = m_tClip.vecEventKeys[m_iSelectedEventKey].fTime - X_To_Time(vMouse.x, fTrackStartX, fTrackWidth);
        }
        else if (m_iHoveredShakeKey >= 0)
        {
            Clear_Selection();
            m_iSelectedShakeKey = m_iHoveredShakeKey;

            auto& t = m_tClip.vecShakeKeys[m_iSelectedShakeKey];
            const _float x0 = Time_To_X(t.fStartTime, fTrackStartX, fTrackWidth);
            const _float x1 = Time_To_X(t.fEndTime, fTrackStartX, fTrackWidth);

            if (fabsf(vMouse.x - x0) <= 6.f)
                m_eDragTarget = TIMELINE_DRAG_TARGET::SHAKE_START;
            else if (fabsf(vMouse.x - x1) <= 6.f)
                m_eDragTarget = TIMELINE_DRAG_TARGET::SHAKE_END;
            else
                m_eDragTarget = TIMELINE_DRAG_TARGET::SHAKE_BODY;

            m_fDragTimeOffset = 0.f;
        }
        else
        {
            Clear_Selection();
            m_eDragTarget = TIMELINE_DRAG_TARGET::PREVIEW;
            m_fDragTimeOffset = 0.f;
            m_fPreviewTime = X_To_Time(vMouse.x, fTrackStartX, fTrackWidth);
            if (m_bAutoPreview)
                Preview_CurrentTime();
        }
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        const _float fMouseTime = X_To_Time(vMouse.x, fTrackStartX, fTrackWidth);

        switch (m_eDragTarget)
        {
        case TIMELINE_DRAG_TARGET::PREVIEW:
            m_fPreviewTime = std::clamp(fMouseTime + m_fDragTimeOffset, 0.f, m_tClip.fDuration);
            if (m_bAutoPreview)
                Preview_CurrentTime();
            break;

        case TIMELINE_DRAG_TARGET::CAMERA:
            if (m_iSelectedCameraKey >= 0)
                m_tClip.vecCameraKeys[m_iSelectedCameraKey].fTime = std::clamp(fMouseTime + m_fDragTimeOffset, 0.f, m_tClip.fDuration);
            break;

        case TIMELINE_DRAG_TARGET::SHOT:
            if (m_iSelectedShotKey >= 0)
                m_tClip.vecShotKeys[m_iSelectedShotKey].fTime = std::clamp(fMouseTime + m_fDragTimeOffset, 0.f, m_tClip.fDuration);
            break;

        case TIMELINE_DRAG_TARGET::EVENT:
            if (m_iSelectedEventKey >= 0)
                m_tClip.vecEventKeys[m_iSelectedEventKey].fTime = std::clamp(fMouseTime + m_fDragTimeOffset, 0.f, m_tClip.fDuration);
            break;

        case TIMELINE_DRAG_TARGET::SHAKE_START:
            if (m_iSelectedShakeKey >= 0)
            {
                auto& t = m_tClip.vecShakeKeys[m_iSelectedShakeKey];
                t.fStartTime = std::clamp(fMouseTime, 0.f, t.fEndTime);
            }
            break;

        case TIMELINE_DRAG_TARGET::SHAKE_END:
            if (m_iSelectedShakeKey >= 0)
            {
                auto& t = m_tClip.vecShakeKeys[m_iSelectedShakeKey];
                t.fEndTime = std::clamp(fMouseTime, t.fStartTime, m_tClip.fDuration);
            }
            break;

        case TIMELINE_DRAG_TARGET::SHAKE_BODY:
            if (m_iSelectedShakeKey >= 0)
            {
                auto& t = m_tClip.vecShakeKeys[m_iSelectedShakeKey];
                const _float fLen = t.fEndTime - t.fStartTime;
                _float fNewStart = std::clamp(fMouseTime - fLen * 0.5f, 0.f, m_tClip.fDuration - fLen);
                t.fStartTime = fNewStart;
                t.fEndTime = fNewStart + fLen;
            }
            break;

        default:
            break;
        }

        if (vMouse.x > vMax.x - 18.f)
            m_fTimelineOffsetTime += 0.03f * Get_VisibleDuration(fTrackWidth);
        else if (vMouse.x < fTrackStartX + 18.f)
            m_fTimelineOffsetTime -= 0.03f * Get_VisibleDuration(fTrackWidth);

        Clamp_TimelineView(fTrackWidth);
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        if (m_eDragTarget != TIMELINE_DRAG_TARGET::PREVIEW)
            Sort_All_Keys();

        m_eDragTarget = TIMELINE_DRAG_TARGET::NONE;
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        m_fPreviewTime = X_To_Time(vMouse.x, fTrackStartX, fTrackWidth);
        Add_CameraKey_From_Selected();
    }
}
