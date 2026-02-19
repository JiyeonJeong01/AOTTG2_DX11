#include "ProfilerPanel.h"
#include <Engine_Log.h>

#include <psapi.h>

NS_BEGIN(Editor)

CProfilerPanel* CProfilerPanel::s_pActive = nullptr;

struct CProfilerPanel::ProfilerRuntime
{
    struct SScopeActive
    {
        const char* name = nullptr;
        clock::time_point t0;
    };

    struct SScopeStatsInternal
    {
        SCOPE_STATS ui;

        std::deque<double> window;
        double window_sum = 0.0;

        uint64_t lastTouchedFrame = 0;
    };

    std::vector<SScopeActive> scopeStack;
    std::unordered_map<std::string, SScopeStatsInternal> scopes;

    uint64_t frameIndex = 0;
    int      avgWindow = 120;
    bool     capture = true;

    bool     externalFrameTime = false;
    double   externalFrameMS = 0.0;

    std::deque<double> frameWin;
    double frameSum = 0.0;
};

static bool Str_IContains(const std::string& hay, const std::string& needle)
{
    if (needle.empty()) return true;

    auto tolow = [](unsigned char c) { return (char)std::tolower(c); };

    std::string h; h.reserve(hay.size());
    std::string n; n.reserve(needle.size());

    for (char c : hay) h.push_back(tolow((unsigned char)c));
    for (char c : needle) n.push_back(tolow((unsigned char)c));

    return (h.find(n) != std::string::npos);
}

static void Push_Window(CProfilerPanel::ProfilerRuntime::SScopeStatsInternal& s, double v, int maxWindow)
{
    s.window.push_back(v);
    s.window_sum += v;

    while ((int)s.window.size() > maxWindow)
    {
        s.window_sum -= s.window.front();
        s.window.pop_front();
    }
}

static double Get_Window_Avg(const CProfilerPanel::ProfilerRuntime::SScopeStatsInternal& s)
{
    if (s.window.empty()) return 0.0;
    return s.window_sum / (double)s.window.size();
}

CProfilerPanel::CProfilerPanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName)
{
}

CProfilerPanel::~CProfilerPanel()
{
    if (s_pActive == this)
        s_pActive = nullptr;
}

HRESULT CProfilerPanel::Initialize()
{
    m_prevFrame = clock::now();
    m_pRuntime = std::make_unique<ProfilerRuntime>();

    // Active panel for static Begin/End routing
    s_pActive = this;

    return S_OK;
}

void CProfilerPanel::Update()
{
    On_Frame_Begin();
    On_Frame_End();
}

void CProfilerPanel::Render()
{
    if (!ImGui::Begin(m_strPanelName.c_str()))
    {
        ImGui::End();
        return;
    }

    Draw_Toolbar();
    ImGui::Separator();
    Draw_Frame_Info();
    ImGui::Separator();
    Draw_Scopes();
    ImGui::Separator();
    Draw_Memory();

    ImGui::End();
}

void CProfilerPanel::Begin_Scope(const char* pszName)
{
    if (!s_pActive) return;
    auto& rt = *s_pActive->m_pRuntime;

    if (!rt.capture) return;
    if (!pszName || !pszName[0]) return;

    ProfilerRuntime::SScopeActive a;
    a.name = pszName;
    a.t0 = clock::now();
    rt.scopeStack.push_back(a);
}

void CProfilerPanel::End_Scope()
{
    if (!s_pActive) return;
    auto& rt = *s_pActive->m_pRuntime;

    if (!rt.capture) return;
    if (rt.scopeStack.empty()) return;

    const auto t1 = clock::now();

    auto a = rt.scopeStack.back();
    rt.scopeStack.pop_back();

    const double ms = std::chrono::duration<double, std::milli>(t1 - a.t0).count();

    auto& entry = rt.scopes[a.name]; // creates if not exists
    auto& s = entry;

    if (!s.ui.bEnabled)
        return;

    if (s.ui.samples == 0)
    {
        s.ui.name = a.name;
        s.ui.min_ms = ms;
        s.ui.max_ms = ms;
        s.ui.bEnabled = true;
    }
    else
    {
        s.ui.min_ms = min(s.ui.min_ms, ms);
        s.ui.max_ms = max(s.ui.max_ms, ms);
    }

    s.ui.last_ms = ms;
    s.ui.samples++;

    Push_Window(s, ms, rt.avgWindow);
    s.ui.avg_ms = Get_Window_Avg(s);

    s.lastTouchedFrame = rt.frameIndex;

}

void CProfilerPanel::Set_Frame_Time_External(double frame_ms)
{
    if (!s_pActive) return;
    auto& rt = *s_pActive->m_pRuntime;

    rt.externalFrameTime = true;
    rt.externalFrameMS = frame_ms;
}

void CProfilerPanel::On_Frame_Begin()
{
    auto& rt = *m_pRuntime;

    rt.frameIndex++;

    rt.avgWindow = max(1, m_iAvgWindow);
    rt.capture = m_bCapture;
}

void CProfilerPanel::On_Frame_End()
{
    auto& rt = *m_pRuntime;

    const auto now = clock::now();

    double frameMS = 0.0;
    if (rt.externalFrameTime)
    {
        frameMS = rt.externalFrameMS;
        rt.externalFrameTime = false;
    }
    else
    {
        frameMS = std::chrono::duration<double, std::milli>(now - m_prevFrame).count();
        m_prevFrame = now;
    }

    m_frameMS_last = frameMS;
    m_fps_last = (frameMS > 0.0001) ? (1000.0 / frameMS) : 0.0;

    // rolling avg (per-runtime)
    rt.frameWin.push_back(frameMS);
    rt.frameSum += frameMS;
    while ((int)rt.frameWin.size() > rt.avgWindow)
    {
        rt.frameSum -= rt.frameWin.front();
        rt.frameWin.pop_front();
    }

    m_frameMS_avg = rt.frameWin.empty() ? 0.0 : (rt.frameSum / (double)rt.frameWin.size());
    m_fps_avg = (m_frameMS_avg > 0.0001) ? (1000.0 / m_frameMS_avg) : 0.0;

    Prune_Dead_Scopes();
}

void CProfilerPanel::Prune_Dead_Scopes()
{
    auto& rt = *m_pRuntime;

    const uint64_t kKeepFrames = (uint64_t)max(60, rt.avgWindow * 10);

    for (auto it = rt.scopes.begin(); it != rt.scopes.end(); )
    {
        const uint64_t age = (rt.frameIndex - it->second.lastTouchedFrame);
        if (age > kKeepFrames)
            it = rt.scopes.erase(it);
        else
            ++it;
    }
}

void CProfilerPanel::Reset_Stats()
{
    auto& rt = *m_pRuntime;

    for (auto& kv : rt.scopes)
    {
        auto& s = kv.second;
        s.ui.last_ms = 0.0;
        s.ui.avg_ms = 0.0;
        s.ui.min_ms = 0.0;
        s.ui.max_ms = 0.0;
        s.ui.samples = 0;

        s.window.clear();
        s.window_sum = 0.0;
        s.lastTouchedFrame = rt.frameIndex;
    }
}

CProfilerPanel::PROGRESS_MEMORY CProfilerPanel::Get_Process_Memory()
{
    PROCESS_MEMORY_COUNTERS_EX pmc{};
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

    PROGRESS_MEMORY out;
    out.workingSetMB = pmc.WorkingSetSize / (1024 * 1024);
    out.privateBytesMB = pmc.PrivateUsage / (1024 * 1024);
    return out;
}

void CProfilerPanel::Draw_Toolbar()
{
    if (ImGui::Checkbox("Capture", &m_bCapture))
        m_pRuntime->capture = m_bCapture;

    ImGui::SameLine();
    if (ImGui::Button("Reset"))
        Reset_Stats();

    ImGui::SameLine();
    ImGui::Checkbox("Sort by cost", &m_bSortByCost);

    ImGui::SameLine();
    ImGui::Checkbox("Show disabled", &m_bShowDisabled);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(140.f);
    ImGui::InputInt("AvgWindow", &m_iAvgWindow);
    if (m_iAvgWindow < 1)    m_iAvgWindow = 1;
    if (m_iAvgWindow > 1000) m_iAvgWindow = 1000;

    ImGui::SameLine();
    ImGui::SetNextItemWidth(220.f);
    ImGui::InputText("Filter", &m_strFilter);
}

void CProfilerPanel::Draw_Frame_Info()
{
    ImGui::Text("FPS (Last / Avg): %d / %d", (_uint)m_fps_last, (_uint)m_fps_avg);
    ImGui::Text("Frame ms (Last / Avg): %.2f / %.2f", (float)m_frameMS_last, (float)m_frameMS_avg);
}

void CProfilerPanel::Draw_Scopes()
{
    auto& rt = *m_pRuntime;

    struct Row
    {
        SCOPE_STATS* p = nullptr;
        double sortKey = 0.0;
    };

    std::vector<Row> rows;
    rows.reserve(rt.scopes.size());

    for (auto& kv : rt.scopes)
    {
        auto& st = kv.second.ui;

        if (!Str_IContains(st.name, m_strFilter))
            continue;

        Row r;
        r.p = &st;
        r.sortKey = m_bSortByCost ? st.avg_ms : 0.0;
        rows.push_back(r);
    }

    if (m_bSortByCost)
    {
        std::sort(rows.begin(), rows.end(),
            [](const Row& a, const Row& b) { return a.sortKey > b.sortKey; });
    }
    else
    {
        std::sort(rows.begin(), rows.end(),
            [](const Row& a, const Row& b) { return a.p->name < b.p->name; });
    }

    if ((int)rows.size() > m_iMaxScopeRows)
        rows.resize((size_t)m_iMaxScopeRows);

    if (ImGui::BeginTable("##ProfilerScopes", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("On", ImGuiTableColumnFlags_WidthFixed, 36.f);
        ImGui::TableSetupColumn("Scope");
        ImGui::TableSetupColumn("Last (ms)");
        ImGui::TableSetupColumn("Avg (ms)");
        ImGui::TableSetupColumn("Min (ms)");
        ImGui::TableSetupColumn("Max (ms)");
        ImGui::TableHeadersRow();

        for (Row& r : rows)
        {
            SCOPE_STATS& s = *r.p;

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(s.name.c_str());
            ImGui::Checkbox("##en", &s.bEnabled);
            ImGui::PopID();

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(s.name.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.3f", (float)s.last_ms);

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%.3f", (float)s.avg_ms);

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%.3f", (float)s.min_ms);

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%.3f", (float)s.max_ms);
        }

        ImGui::EndTable();
    }
}

void CProfilerPanel::Draw_Memory()
{
    auto mem = Get_Process_Memory();

    ImGui::Text("Memory");
    ImGui::BulletText("Working Set : %zu MB", mem.workingSetMB);
    ImGui::BulletText("Private     : %zu MB", mem.privateBytesMB);
}

std::unique_ptr<CProfilerPanel> CProfilerPanel::Create(const std::string& strPanelName)
{
    auto pInstance = std::make_unique<CProfilerPanel>(strPanelName);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "CProfilerPanel Create failed");
    return pInstance;
}

NS_END
