#pragma once

#include "EditorPanel.h"

NS_BEGIN(Editor)

class CProfilerPanel : public CEditorPanel
{
public:
    CProfilerPanel(const std::string& strPanelName);
    ~CProfilerPanel() override;

public:
    HRESULT Initialize() override;
    void Update() override;
    void Render() override;

public:
    typedef struct tagScopeStats
    {
        std::string name;

        _double last_ms = 0.0;
        _double avg_ms = 0.0;
        _double min_ms = 0.0;
        _double max_ms = 0.0;

        uint64_t samples = 0;
        _bool bEnabled = true;
    } SCOPE_STATS;

    static void Begin_Scope(const char* pszName);
    static void End_Scope();

    class CScope
    {
    public:
        explicit CScope(const char* pszName) { Begin_Scope(pszName); }
        ~CScope() { End_Scope(); }
    };

    static void Set_Frame_Time_External(double frame_ms);

    typedef struct tagProcessMemory
    {
        size_t workingSetMB = 0;
        size_t privateBytesMB = 0;
    } PROGRESS_MEMORY;

    PROGRESS_MEMORY Get_Process_Memory();

private:
    void Draw_Toolbar();
    void Draw_Frame_Info();
    void Draw_Scopes();
    void Draw_Memory();

    void On_Frame_Begin();
    void On_Frame_End();

    void Reset_Stats();
    void Prune_Dead_Scopes();

private:
    std::string m_strFilter;
    bool        m_bCapture = true;
    bool        m_bSortByCost = true;
    bool        m_bShowDisabled = false;

    int         m_iAvgWindow = 120;
    int         m_iMaxScopeRows = 256;

public:
    using clock = std::chrono::steady_clock;
    clock::time_point m_prevFrame = {};

private:
    double      m_frameMS_last = 0.0;
    double      m_frameMS_avg = 0.0;
    double      m_fps_last = 0.0;
    double      m_fps_avg = 0.0;
    const float TARGET_FPS = 60.0f;
    const float FRAME_BUDGET_MS = 1000.0f / TARGET_FPS;

public:
    static std::unique_ptr<CProfilerPanel> Create(const std::string& strPanelName);

    struct ProfilerRuntime;
    std::unique_ptr<ProfilerRuntime> m_pRuntime;

    static CProfilerPanel* s_pActive;
};

NS_END
