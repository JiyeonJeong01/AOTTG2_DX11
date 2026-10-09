#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

/**
 * \brief 프레임 구간 측정기.
 * CPU 구간은 이번 프레임 값이고, GPU 구간은 타임스탬프 쿼리라 몇 프레임 늦게 도착한다.
 * GPU 결과를 기다리면 CPU가 멈추므로, 준비된 프레임만 수거한다.
 * 구간 이름은 문자열 리터럴만 넘긴다 (포인터를 그대로 보관).
 */
class ENGINE_DLL CProfiler final
{
    DECLARE_SINGLETON(CProfiler)

public:
    typedef struct tagSample
    {
        const _char* szName = nullptr;
        _double      dMS = 0.0;
    } SAMPLE;

    HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

    void    Begin_Frame();
    void    End_Frame();

    void    Begin_CPU(const _char* szName);
    void    End_CPU();
    void    Begin_GPU(const _char* szName);
    void    End_GPU();

    /* 시간이 아닌 개수 (드로우 수, 충돌 후보 수 등). 프레임마다 다시 기록 */
    void    Set_Count(const _char* szName, _double dValue);
    const vector<SAMPLE>& Get_Counts() const { return m_Counts; }

    /* 이번 프레임 CPU 구간 */
    const vector<SAMPLE>& Get_CPU_Samples() const { return m_CPUSamples; }
    /* 측정한 GPU 이름 (UTF-8). 내장/외장 결과를 구분하는 용도 */
    const string& Get_Adapter_Name() const { return m_strAdapter; }

    /* 이번 Begin_Frame에서 새로 도착한 프레임의 GPU 구간 (없으면 비어 있음) */
    const vector<SAMPLE>& Get_GPU_Samples() const { return m_GPUSamples; }

private:
    void    Collect_GPU();

private:
    static constexpr _uint FRAME_LATENCY  = 4;
    static constexpr _uint MAX_GPU_SCOPES = 32;
    static constexpr _uint INVALID_SCOPE  = ~0u;

    struct GPU_FRAME
    {
        Microsoft::WRL::ComPtr<ID3D11Query> pDisjoint;
        Microsoft::WRL::ComPtr<ID3D11Query> pBegin[MAX_GPU_SCOPES];
        Microsoft::WRL::ComPtr<ID3D11Query> pEnd[MAX_GPU_SCOPES];
        const _char* szNames[MAX_GPU_SCOPES]{};
        _uint        iCount = 0;
        _bool        bPending = false;
    };

    struct CPU_OPEN
    {
        const _char*                          szName;
        std::chrono::steady_clock::time_point t0;
    };

    ID3D11DeviceContext*    m_pContext = nullptr;
    _bool                   m_bInFrame = false;
    string                  m_strAdapter;

    GPU_FRAME               m_Frames[FRAME_LATENCY];
    _uint                   m_iWrite = 0;           /* 다음에 기록할 프레임 = 가장 오래된 프레임 */
    GPU_FRAME*              m_pCur = nullptr;       /* GPU가 너무 밀려 재사용할 칸이 없으면 nullptr */
    vector<_uint>           m_GPUStack;

    vector<CPU_OPEN>        m_CPUStack;
    vector<SAMPLE>          m_CPUSamples;
    vector<SAMPLE>          m_GPUSamples;
    vector<SAMPLE>          m_Counts;
};

struct CProfile_CPU_Scope
{
    explicit CProfile_CPU_Scope(const _char* szName) { CProfiler::GetInstance().Begin_CPU(szName); }
    ~CProfile_CPU_Scope() { CProfiler::GetInstance().End_CPU(); }
};

struct CProfile_GPU_Scope
{
    explicit CProfile_GPU_Scope(const _char* szName) { CProfiler::GetInstance().Begin_GPU(szName); }
    ~CProfile_GPU_Scope() { CProfiler::GetInstance().End_GPU(); }
};

NS_END

#define PROFILE_CONCAT_INNER(a, b) a##b
#define PROFILE_CONCAT(a, b)       PROFILE_CONCAT_INNER(a, b)
#define PROFILE_CPU(name)          Engine::CProfile_CPU_Scope PROFILE_CONCAT(_profileCPU, __LINE__)(name)
#define PROFILE_GPU(name)          Engine::CProfile_GPU_Scope PROFILE_CONCAT(_profileGPU, __LINE__)(name)
