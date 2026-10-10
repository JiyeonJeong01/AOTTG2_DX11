#include "ProfilerPanel.h"
#include <Engine_Log.h>
#include <Profiler.h>
#include <Component_System.h>
#include <CRender_System.h>
#include <MeshRenderer_Processor.h>
#include <Physics_Processor.h>
#include <Component_System.inl>
#include <Transform.h>

#include <psapi.h>

NS_BEGIN(Editor)

CProfilerPanel* CProfilerPanel::s_pActive = nullptr;
bool            CProfilerPanel::s_bUncapFPS = false; /* GPU 측정할 때만 켠다 (켜면 GPU 클럭이 안정됨) */

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

    Add_Sample(a.name, ms);
}

void CProfilerPanel::Feed_Engine_Samples()
{
    if (!s_pActive || !s_pActive->m_pRuntime->capture) return;

    for (const auto& tSample : SYS_PROFILER.Get_CPU_Samples())
        Add_Sample(tSample.szName, tSample.dMS);

    for (const auto& tSample : SYS_PROFILER.Get_GPU_Samples())
        Add_Sample(std::string("GPU/") + tSample.szName, tSample.dMS);

    /* '#' 항목은 ms가 아니라 개수 */
    for (const auto& tCount : SYS_PROFILER.Get_Counts())
        Add_Sample(std::string("#") + tCount.szName, tCount.dMS);
}

void CProfilerPanel::Add_Sample(const std::string& strName, double ms)
{
    if (!s_pActive) return;
    auto& rt = *s_pActive->m_pRuntime;

    auto& s = rt.scopes[strName]; // creates if not exists

    if (!s.ui.bEnabled)
        return;

    if (s.ui.samples == 0)
    {
        s.ui.name = strName;
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

/* 측정 조건 + 비용순 표를 텍스트로 복사 (결과 공유용) */
void CProfilerPanel::Copy_To_Clipboard()
{
    auto& rt = *m_pRuntime;

    std::vector<const SCOPE_STATS*> rows;
    for (auto& kv : rt.scopes)
        rows.push_back(&kv.second.ui);
    std::sort(rows.begin(), rows.end(),
        [](const SCOPE_STATS* a, const SCOPE_STATS* b) { return a->avg_ms > b->avg_ms; });

#ifdef _DEBUG
    const char* pszConfig = "Debug";
#else
    const char* pszConfig = "Release";
#endif
    const auto mem = Get_Process_Memory();

    std::string out;
    char line[256];

    const ImVec2 vDisplay = ImGui::GetIO().DisplaySize;
    snprintf(line, sizeof(line), "GPU: %s | %s | Window %.0fx%.0f | Render cap: %s (update 60Hz) | AvgWindow: %d\n",
        SYS_PROFILER.Get_Adapter_Name().c_str(), pszConfig,
        vDisplay.x, vDisplay.y, s_bUncapFPS ? "off" : "60", m_iAvgWindow);
    out += line;
    snprintf(line, sizeof(line), "FPS avg %.0f | Frame avg %.2f ms | Memory %zu MB\n",
        m_fps_avg, m_frameMS_avg, mem.privateBytesMB);
    out += line;

    auto* pMesh = SYS_COMPONENT.Bind_Processor<CMeshRenderer_Processor>();
    snprintf(line, sizeof(line), "Switches: FrustumCull %s | StaticShadowCache %s | CompactShadowRT %s  ('#' rows are counts, not ms)\n",
        (pMesh && pMesh->Is_FrustumCull()) ? "on" : "off",
        SYS_RENDER.Is_StaticShadowCache() ? "on" : "off",
        SYS_RENDER.Is_CompactShadowRT() ? "on" : "off");
    out += line;

    if (auto* pPhysics = SYS_COMPONENT.Bind_Processor<CPhysics_Processor>())
    {
        snprintf(line, sizeof(line), "Physics: SolverIter %u | PenetrationOnce %s\n",
            pPhysics->Get_SolverIterations(), pPhysics->Is_PenetrationOnce() ? "on" : "off");
        out += line;
    }
    snprintf(line, sizeof(line), "%-28s %9s %9s %9s\n", "Scope", "Avg(ms)", "Min(ms)", "Max(ms)");
    out += line;

    for (const SCOPE_STATS* s : rows)
    {
        snprintf(line, sizeof(line), "%-28s %9.3f %9.3f %9.3f\n", s->name.c_str(), s->avg_ms, s->min_ms, s->max_ms);
        out += line;
    }

    ImGui::SetClipboardText(out.c_str());
}

/*
    측정용: 컴포넌트 접근 방식 비교 (결과는 클립보드로)
    1) 정확성: 해제 후 같은 칸이 재사용될 때, 예전 Is_Valid 로직과 현재 Is_Valid가 각각 무엇을 돌려주는지
    2) 성능: 핸들로 매번 조회 / 캐싱한 포인터 / 포인터 + Is_Valid 의 접근 1회당 비용
    실제 Transform 컴포넌트를 만들어 쓰고, 끝나기 전에 전부 제거한다.
*/
void CProfilerPanel::Run_Proxy_Test()
{
    using clk = std::chrono::steady_clock;
    const OBJECT_HANDLE hDummy{};
    std::string out;
    char line[256];

#ifdef _DEBUG
    out += "Config: Debug (measure timings in Release)\n"; /* 출력은 영어: 한글 리터럴은 실행 문자셋(CP949)으로 들어가 클립보드에서 깨진다 */
#else
    out += "Config: Release\n";
#endif

    /* ---------- 1) 정확성: 해제 후 칸 재사용 ---------- */
    {
        const COMPONENT_HANDLE h1 = SYS_COMPONENT.Create_Component_By_Type(COMPONENT_TYPE::TRANSFORM, hDummy);
        CTransform tOld = SYS_COMPONENT.Get_Proxy<CTransform>(COMPONENT_TYPE::TRANSFORM, h1);

        SYS_COMPONENT.Remove_Component_By_Type(COMPONENT_TYPE::TRANSFORM, h1);
        const COMPONENT_HANDLE h2 = SYS_COMPONENT.Create_Component_By_Type(COMPONENT_TYPE::TRANSFORM, hDummy); /* 빈 칸 목록은 LIFO → 같은 칸 재사용 */
        CTransform tNew = SYS_COMPONENT.Get_Proxy<CTransform>(COMPONENT_TYPE::TRANSFORM, h2);

        const bool bSameSlot = (tOld._Data() == tNew._Data());
        const bool bLegacyValid = (tOld._Data() != nullptr && tOld.Get_Handle().Is_Valid()); /* 예전 Is_Valid 로직 */
        const bool bCurrentValid = tOld.Is_Valid();

        out += "[1] Stale proxy after slot reuse\n";
        snprintf(line, sizeof(line), "  same slot reused      : %s (old handle v%u, new handle v%u)\n",
            bSameSlot ? "yes" : "no", tOld.Get_Handle().Get_Version(), h2.Get_Version());
        out += line;
        snprintf(line, sizeof(line), "  legacy Is_Valid (old) : %s  <- freed component reported as valid\n", bLegacyValid ? "true" : "false");
        out += line;
        snprintf(line, sizeof(line), "  current Is_Valid (old): %s\n", bCurrentValid ? "true" : "false");
        out += line;

        SYS_COMPONENT.Remove_Component_By_Type(COMPONENT_TYPE::TRANSFORM, h2);
    }

    /* ---------- 2) 성능: 접근 방식별 비용 ---------- */
    {
        constexpr int N = 10000;
        constexpr int ROUNDS = 100;

        std::vector<COMPONENT_HANDLE> vecHandles;
        std::vector<CTransform>       vecProxies;
        vecHandles.reserve(N);
        vecProxies.reserve(N);
        for (int i = 0; i < N; ++i)
        {
            vecHandles.push_back(SYS_COMPONENT.Create_Component_By_Type(COMPONENT_TYPE::TRANSFORM, hDummy));
            vecProxies.push_back(SYS_COMPONENT.Get_Proxy<CTransform>(COMPONENT_TYPE::TRANSFORM, vecHandles.back()));
        }

        auto Measure = [&](auto&& fnAccess) -> double
        {
            const auto t0 = clk::now();
            for (int r = 0; r < ROUNDS; ++r)
                for (int i = 0; i < N; ++i)
                    fnAccess(i);
            const double ns = std::chrono::duration<double, std::nano>(clk::now() - t0).count();
            return ns / (double(N) * ROUNDS);
        };

        float fSink = 0.f; /* 최적화로 루프가 사라지지 않도록 결과를 모은다 */

        const double nsHandle = Measure([&](int i)
        {
            CTransform tr = SYS_COMPONENT.Get_Proxy<CTransform>(COMPONENT_TYPE::TRANSFORM, vecHandles[i]);
            tr->vPosition.x += 1.f;
            fSink += tr->vPosition.x;
        });
        const double nsPointer = Measure([&](int i)
        {
            vecProxies[i]->vPosition.x += 1.f;
            fSink += vecProxies[i]->vPosition.x;
        });
        const double nsChecked = Measure([&](int i)
        {
            if (!vecProxies[i].Is_Valid())
                return;
            vecProxies[i]->vPosition.x += 1.f;
            fSink += vecProxies[i]->vPosition.x;
        });

        snprintf(line, sizeof(line), "[2] Access cost (%d components x %d rounds, ns per access)\n", N, ROUNDS);
        out += line;
        snprintf(line, sizeof(line), "  handle lookup every access : %7.2f ns\n", nsHandle);
        out += line;
        snprintf(line, sizeof(line), "  cached pointer             : %7.2f ns\n", nsPointer);
        out += line;
        snprintf(line, sizeof(line), "  cached pointer + Is_Valid  : %7.2f ns\n", nsChecked);
        out += line;
        snprintf(line, sizeof(line), "  (checksum %.0f)\n", fSink);
        out += line;

        for (const COMPONENT_HANDLE& h : vecHandles)
            SYS_COMPONENT.Remove_Component_By_Type(COMPONENT_TYPE::TRANSFORM, h);
    }

    ImGui::SetClipboardText(out.c_str());
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
    if (ImGui::Button("Copy"))
        Copy_To_Clipboard();

    ImGui::SameLine();
    if (ImGui::Button("Proxy Test"))
        Run_Proxy_Test();

    ImGui::SameLine();
    ImGui::Checkbox("Sort by cost", &m_bSortByCost);

    ImGui::SameLine();
    ImGui::Checkbox("Show disabled", &m_bShowDisabled);

    ImGui::SameLine();
    ImGui::Checkbox("Uncap Render", &s_bUncapFPS); /* 그리기만 제한 해제. 게임 갱신은 60Hz 그대로 */

    /* 측정용 최적화 스위치: 끈 상태와 켠 상태를 같은 장면에서 비교 */
    ImGui::TextUnformatted("Optimizations:");

    if (auto* pMesh = SYS_COMPONENT.Bind_Processor<CMeshRenderer_Processor>())
    {
        _bool bOn = pMesh->Is_FrustumCull();
        ImGui::SameLine();
        if (ImGui::Checkbox("Frustum Cull", &bOn))
            pMesh->Set_FrustumCull(bOn);
    }
    {
        _bool bOn = SYS_RENDER.Is_StaticShadowCache();
        ImGui::SameLine();
        if (ImGui::Checkbox("Static Shadow Cache", &bOn))
            SYS_RENDER.Set_StaticShadowCache(bOn);

        _bool bCompact = SYS_RENDER.Is_CompactShadowRT();
        ImGui::SameLine();
        if (ImGui::Checkbox("Compact Shadow RT", &bCompact))
            SYS_RENDER.Set_CompactShadowRT(bCompact);
    }

    /* Solver 비교 : 반복 횟수 1/4/8, 침투 보정을 반복마다 vs 반복 뒤 한 번 */
    if (auto* pPhysics = SYS_COMPONENT.Bind_Processor<CPhysics_Processor>())
    {
        ImGui::TextUnformatted("Physics:");
        ImGui::SameLine();
        _int iIter = (_int)pPhysics->Get_SolverIterations();
        ImGui::SetNextItemWidth(100.f);
        if (ImGui::InputInt("Solver Iter", &iIter))
            pPhysics->Set_SolverIterations((_uint)std::clamp(iIter, 1, 16));

        ImGui::SameLine();
        _bool bOnce = pPhysics->Is_PenetrationOnce();
        if (ImGui::Checkbox("Penetration Once", &bOnce))
            pPhysics->Set_PenetrationOnce(bOnce);
    }

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
