#pragma once

#include "EditorPanel.h"
#include "Cinematic_Define.h"

namespace Engine
{
    class CGameObject;
}

NS_BEGIN(Editor)
    class CHierarchyPanel;

class CCinematicPanel final : public CEditorPanel
{
public:
    CCinematicPanel(const std::string& strPanelName);

public:
    HRESULT Initialize(CHierarchyPanel* pPanel);
    static std::unique_ptr<CCinematicPanel> Create(const std::string& strPanelName, CHierarchyPanel* pHierarchy);

public:
    void Set_Target(class Engine::CGameObject* pObj);
    void Render() override;

private:
    void Draw_Toolbar();
    void Draw_Workspace();

    void Draw_InspectorPanel();
    void Draw_TimelinePanel();

    void Draw_CameraKey_Inspector();
    void Draw_ShotKey_Inspector();
    void Draw_EventKey_Inspector();
    void Draw_ShakeKey_Inspector();

    void Draw_TimelineCanvas(const ImVec2& vCanvasSize);
    void Draw_TimelineTopBar();
    void Draw_TrackRowLabel(const char* pLabel, _float fY, ImDrawList* pDraw, const ImVec2& vMin, const ImVec2& vMax);
    void Draw_TimeRuler(ImDrawList* pDraw, const ImVec2& vMin, const ImVec2& vMax, _float fTrackStartX, _float fTrackWidth);
    void Handle_TimelineInput(const ImVec2& vMin, const ImVec2& vMax, _float fTrackStartX, _float fTrackWidth);

    void Add_CameraKey_From_Selected();
    void Add_ShotKey();
    void Add_EventKey();
    void Add_ShakeKey();

    void Remove_Selected();
    void Sort_All_Keys();
    void Clear_Selection();

    void Preview_CurrentTime();
    void Capture_Selected_CameraKey_From_Target();
    void Apply_Selected_CameraKey_To_Target();

    _float Time_To_X(_float fTime, _float fStartX, _float fWidth) const;
    _float X_To_Time(_float fX, _float fStartX, _float fWidth) const;

    void Focus_Time(_float fTime);
    void Clamp_TimelineView(_float fTrackWidth);
    _float Get_PixelsPerSecond() const;
    _float Get_VisibleDuration(_float fTrackWidth) const;
    void Clamp_All_Key_Time();

private:
    enum class TIMELINE_DRAG_TARGET : uint8_t
    {
        NONE,
        PREVIEW,
        CAMERA,
        SHOT,
        EVENT,
        SHAKE_BODY,
        SHAKE_START,
        SHAKE_END
    };

private:
    Engine::CGameObject* m_pTarget = nullptr;

    CINEMATIC_CLIP m_tClip{};

    _float m_fPreviewTime = 0.f;
    _bool  m_bAutoPreview = true;

    int m_iSelectedCameraKey = -1;
    int m_iSelectedShotKey = -1;
    int m_iSelectedEventKey = -1;
    int m_iSelectedShakeKey = -1;

    _float3 m_vCachedCameraKeyEuler{ 0.f, 0.f, 0.f };
    int     m_iCachedCameraKeyEulerIndex = -1;

private:
    _float m_fInspectorWidth = 360.f;

    _float m_fTimelineZoom = 1.f;
    _float m_fTimelineOffsetTime = 0.f;

    TIMELINE_DRAG_TARGET m_eDragTarget = TIMELINE_DRAG_TARGET::NONE;
    _float m_fDragTimeOffset = 0.f;

    int m_iHoveredCameraKey = -1;
    int m_iHoveredShotKey = -1;
    int m_iHoveredEventKey = -1;
    int m_iHoveredShakeKey = -1;
};

NS_END
