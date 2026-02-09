#include "ProfilerPanel.h"

#include <Engine_Log.h>

NS_BEGIN(Editor)
    namespace
{
    /* ---- Global profiler storage ---- */

    struct SScopeActive
    {
        const char* name = nullptr;
        CProfilerPanel::clock::time_point t0;
    };

    struct SScopeStatsInternal
    {
        CProfilerPanel::SCOPE_STATS ui;

        /* rolling average over last N samples (frame window) */
        std::deque<double> window;
        double window_sum = 0.0;

        /* housekeeping */
        uint64_t lastTouchedFrame = 0;
    };

//    static thread_local std::vector<SScopeActive> g_tlsStack;

    //static std::unordered_map<std::string, SScopeStatsInternal> g_scopes;

    static uint64_t g_frameIndex = 0;

    static int g_avgWindow = 120;
    static bool g_capture = true;

    static bool g_externalFrameTime = false;
    static double g_externalFrameMS = 0.0;

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

    static void Push_Window(SScopeStatsInternal& s, double v)
    {
        s.window.push_back(v);
        s.window_sum += v;

        while ((int)s.window.size() > g_avgWindow)
        {
            s.window_sum -= s.window.front();
            s.window.pop_front();
        }
    }

    static double Get_Window_Avg(const SScopeStatsInternal& s)
    {
        if (s.window.empty()) return 0.0;
        return s.window_sum / (double)s.window.size();
    }
}

CProfilerPanel::CProfilerPanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName)
{
}

CProfilerPanel::~CProfilerPanel()
{
}

HRESULT CProfilerPanel::Initialize()
{
    m_prevFrame = clock::now();

    /* Sync globals */
    g_avgWindow = max(1, m_iAvgWindow);
    g_capture = m_bCapture;

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

void CProfilerPanel::Set_Frame_Time_External(double frame_ms)
{
    g_externalFrameTime = true;
    g_externalFrameMS = frame_ms;
}

/* ---------------- Recording API ---------------- */

void CProfilerPanel::Begin_Scope(const char* pszName)
{
    if (!g_capture) return;
    if (!pszName || !pszName[0]) return;

    SScopeActive a;
    a.name = pszName;
    a.t0 = clock::now();
    //g_tlsStack.push_back(a);
}

void CProfilerPanel::End_Scope()
{
    //if (!g_capture) return;
    //if (g_tlsStack.empty()) return;

    //const auto t1 = clock::now();

    //SScopeActive a = g_tlsStack.back();
    //g_tlsStack.pop_back();

    //const double ms = std::chrono::duration<double, std::milli>(t1 - a.t0).count();

    //auto it = g_scopes.find(a.name);
    //if (it == g_scopes.end())
    //{
    //    SScopeStatsInternal s;
    //    s.ui.name = a.name;
    //    s.ui.last_ms = ms;
    //    s.ui.avg_ms = ms;
    //    s.ui.min_ms = ms;
    //    s.ui.max_ms = ms;
    //    s.ui.samples = 1;
    //    s.ui.bEnabled = true;

    //    Push_Window(s, ms);
    //    s.ui.avg_ms = Get_Window_Avg(s);

    //    s.lastTouchedFrame = g_frameIndex;

    //    g_scopes.emplace(s.ui.name, std::move(s));
    //}
    //else
    //{
    //    SScopeStatsInternal& s = it->second;

    //    s.ui.last_ms = ms;
    //    s.ui.samples++;

    //    if (s.ui.samples == 1)
    //    {
    //        s.ui.min_ms = ms;
    //        s.ui.max_ms = ms;
    //    }
    //    else
    //    {
    //        s.ui.min_ms = min(s.ui.min_ms, ms);
    //        s.ui.max_ms = max(s.ui.max_ms, ms);
    //    }

    //    Push_Window(s, ms);
    //    s.ui.avg_ms = Get_Window_Avg(s);

    //    s.lastTouchedFrame = g_frameIndex;
    //}
}

/* ---------------- Internals ---------------- */

void CProfilerPanel::On_Frame_Begin()
{
    ++g_frameIndex;

    /* apply UI-config -> global */
    g_avgWindow = max(1, m_iAvgWindow);
    g_capture = m_bCapture;
}

void CProfilerPanel::On_Frame_End()
{
    const auto now = clock::now();

    double frameMS = 0.0;
    if (g_externalFrameTime)
    {
        frameMS = g_externalFrameMS;
        g_externalFrameTime = false; /* one-shot by default */
    }
    else
    {
        frameMS = std::chrono::duration<double, std::milli>(now - m_prevFrame).count();
        m_prevFrame = now;
    }

    m_frameMS_last = frameMS;
    m_fps_last = (frameMS > 0.0001) ? (1000.0 / frameMS) : 0.0;

    /* rolling avg */
    static std::deque<double> s_frameWin;
    static double s_frameSum = 0.0;

    s_frameWin.push_back(frameMS);
    s_frameSum += frameMS;
    while ((int)s_frameWin.size() > g_avgWindow)
    {
        s_frameSum -= s_frameWin.front();
        s_frameWin.pop_front();
    }

    m_frameMS_avg = s_frameWin.empty() ? 0.0 : (s_frameSum / (double)s_frameWin.size());
    m_fps_avg = (m_frameMS_avg > 0.0001) ? (1000.0 / m_frameMS_avg) : 0.0;

    Prune_Dead_Scopes();
}

void CProfilerPanel::Prune_Dead_Scopes()
{
    /* prune scopes not touched for long time to keep panel clean */
    const uint64_t kKeepFrames = (uint64_t)max(60, g_avgWindow * 10);

    //for (auto it = g_scopes.begin(); it != g_scopes.end(); )
    //{
    //    const uint64_t age = (g_frameIndex - it->second.lastTouchedFrame);
    //    if (age > kKeepFrames)
    //        it = g_scopes.erase(it);
    //    else
    //        ++it;
    //}
}

void CProfilerPanel::Reset_Stats()
{
    //for (auto& kv : g_scopes)
    //{
    //    auto& s = kv.second;
    //    s.ui.last_ms = 0.0;
    //    s.ui.avg_ms = 0.0;
    //    s.ui.min_ms = 0.0;
    //    s.ui.max_ms = 0.0;
    //    s.ui.samples = 0;

    //    s.window.clear();
    //    s.window_sum = 0.0;
    //    s.lastTouchedFrame = g_frameIndex;
    //}
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
        g_capture = m_bCapture;

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
    if (m_iAvgWindow < 1) m_iAvgWindow = 1;
    if (m_iAvgWindow > 1000) m_iAvgWindow = 1000;

    ImGui::SameLine();
    ImGui::SetNextItemWidth(220.f);
    ImGui::InputText("Filter", &m_strFilter);
}

void CProfilerPanel::Draw_Frame_Info()
{
    ImGui::Text("FPS (Last / Avg): %d / %d", (_uint)m_fps_last, (_uint)m_fps_avg);
    ImGui::Text("Frame ms (Last / Avg): %.1f / %.1f", (float)m_frameMS_last, (float)m_frameMS_avg);

    /* A tiny “looks cool” bar, still simple */

    const float ms = (float)m_frameMS_last;
    float norm = ms / FRAME_BUDGET_MS;

    if (norm < 0.f) norm = 0.f;
    if (norm > 2.f) norm = 2.f;
}

void CProfilerPanel::Draw_Scopes()
{
    ///* collect rows */
    //struct Row
    //{
    //    const CProfilerPanel::SCOPE_STATS* p = nullptr;
    //    double sortKey = 0.0;
    //};

    //std::vector<Row> rows;
    //rows.reserve(g_scopes.size());

    //for (auto& kv : g_scopes)
    //{
    //    auto& st = kv.second.ui;

    //    if (!m_bShowDisabled && !kv.second.ui.bEnabled)
    //        continue;

    //    if (!Str_IContains(st.name, m_strFilter))
    //        continue;

    //    Row r;
    //    r.p = &st;
    //    r.sortKey = m_bSortByCost ? st.avg_ms : 0.0;
    //    rows.push_back(r);
    //}

    //if (m_bSortByCost)
    //{
    //    std::sort(rows.begin(), rows.end(),
    //        [](const Row& a, const Row& b) { return a.sortKey > b.sortKey; });
    //}
    //else
    //{
    //    std::sort(rows.begin(), rows.end(),
    //        [](const Row& a, const Row& b) { return a.p->name < b.p->name; });
    //}

    //if ((int)rows.size() > m_iMaxScopeRows)
    //    rows.resize((size_t)m_iMaxScopeRows);

    //if (ImGui::BeginTable("##ProfilerScopes", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
    //{
    //    ImGui::TableSetupColumn("On", ImGuiTableColumnFlags_WidthFixed, 36.f);
    //    ImGui::TableSetupColumn("Scope");
    //    ImGui::TableSetupColumn("Last (ms)");
    //    ImGui::TableSetupColumn("Avg (ms)");
    //    ImGui::TableSetupColumn("Min (ms)");
    //    ImGui::TableSetupColumn("Max (ms)");
    //    ImGui::TableHeadersRow();

    //    for (Row& r : rows)
    //    {
    //        const SCOPE_STATS& s = *r.p;

    //        ImGui::TableNextRow();

    //        /* On */
    //        ImGui::TableSetColumnIndex(0);
    //        {
    //            bool bEnabled = s.bEnabled;

    //            /* We need to toggle the real stored flag: find by name */
    //            auto it = g_scopes.find(s.name);
    //            if (it != g_scopes.end())
    //            {
    //                bEnabled = it->second.ui.bEnabled;
    //                ImGui::PushID(it->second.ui.name.c_str());
    //                if (ImGui::Checkbox("##en", &bEnabled))
    //                    it->second.ui.bEnabled = bEnabled;
    //                ImGui::PopID();
    //            }
    //            else
    //            {
    //                ImGui::TextUnformatted("-");
    //            }
    //        }

    //        ImGui::TableSetColumnIndex(1);
    //        ImGui::TextUnformatted(s.name.c_str());

    //        ImGui::TableSetColumnIndex(2);
    //        ImGui::Text("%.3f", (float)s.last_ms);

    //        ImGui::TableSetColumnIndex(3);
    //        ImGui::Text("%.3f", (float)s.avg_ms);

    //        ImGui::TableSetColumnIndex(4);
    //        ImGui::Text("%.3f", (float)s.min_ms);

    //        ImGui::TableSetColumnIndex(5);
    //        ImGui::Text("%.3f", (float)s.max_ms);
    //    }

    //    ImGui::EndTable();
    //}

    //ImGui::Spacing();
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
    if (FAILED(pInstance->Initialize()))
    {
        _DEBUG_ERROR_BREAK("CProfilerPanel Create failed");
        return nullptr;
    }
    return pInstance;
}

NS_END
