#include "Profiler.h"
#include "Engine_Log.h"

NS_BEGIN(Engine)

IMPLEMENT_SINGLETON(CProfiler)

CProfiler::CProfiler() = default;
CProfiler::~CProfiler() = default;

HRESULT CProfiler::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    IF_NULL_RETURN_MSG_BREAK(pDevice, E_FAIL, "Profiler: pDevice is nullptr");
    IF_NULL_RETURN_MSG_BREAK(pContext, E_FAIL, "Profiler: pContext is nullptr");

    m_pContext = pContext;

    Microsoft::WRL::ComPtr<IDXGIDevice>  pDXGIDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter;
    DXGI_ADAPTER_DESC tAdapterDesc{};
    if (SUCCEEDED(pDevice->QueryInterface(IID_PPV_ARGS(&pDXGIDevice))) &&
        SUCCEEDED(pDXGIDevice->GetAdapter(&pAdapter)) &&
        SUCCEEDED(pAdapter->GetDesc(&tAdapterDesc)))
    {
        const _int iLen = WideCharToMultiByte(CP_UTF8, 0, tAdapterDesc.Description, -1, nullptr, 0, nullptr, nullptr);
        if (iLen > 1)
        {
            m_strAdapter.resize(iLen - 1);
            WideCharToMultiByte(CP_UTF8, 0, tAdapterDesc.Description, -1, m_strAdapter.data(), iLen, nullptr, nullptr);
        }
    }

    D3D11_QUERY_DESC tDisjoint{ D3D11_QUERY_TIMESTAMP_DISJOINT, 0 };
    D3D11_QUERY_DESC tStamp{ D3D11_QUERY_TIMESTAMP, 0 };

    for (GPU_FRAME& tFrame : m_Frames)
    {
        IF_FAIL_RETURN_MSG_BREAK(pDevice->CreateQuery(&tDisjoint, &tFrame.pDisjoint), E_FAIL, "Profiler: disjoint query failed");

        for (_uint i = 0; i < MAX_GPU_SCOPES; ++i)
        {
            IF_FAIL_RETURN_MSG_BREAK(pDevice->CreateQuery(&tStamp, &tFrame.pBegin[i]), E_FAIL, "Profiler: timestamp query failed");
            IF_FAIL_RETURN_MSG_BREAK(pDevice->CreateQuery(&tStamp, &tFrame.pEnd[i]), E_FAIL, "Profiler: timestamp query failed");
        }
    }

    return S_OK;
}

void CProfiler::Begin_Frame()
{
    m_CPUSamples.clear();
    m_Counts.clear();
    m_CPUStack.clear();
    m_GPUStack.clear();
    m_bInFrame = true;

    if (!m_pContext)
        return;

    Collect_GPU();

    GPU_FRAME& tFrame = m_Frames[m_iWrite];
    if (tFrame.bPending) /* GPU가 FRAME_LATENCY 이상 밀림: 이번 프레임 GPU 측정은 건너뜀 */
    {
        m_pCur = nullptr;
        return;
    }

    tFrame.iCount = 0;
    m_pCur = &tFrame;
    m_pContext->Begin(tFrame.pDisjoint.Get());

    Begin_GPU("Frame");
}

void CProfiler::End_Frame()
{
    m_bInFrame = false;

    if (!m_pCur)
        return;

    while (!m_GPUStack.empty()) /* "Frame" + 닫히지 않은 구간 */
        End_GPU();

    m_pContext->End(m_pCur->pDisjoint.Get());
    m_pCur->bPending = true;
    m_pCur = nullptr;

    m_iWrite = (m_iWrite + 1) % FRAME_LATENCY;
}

void CProfiler::Begin_CPU(const _char* szName)
{
    if (!m_bInFrame) /* 프레임 경계를 안 부르는 실행 파일(Game.exe)에서 샘플이 쌓이지 않도록 */
        return;

    m_CPUStack.push_back({ szName, std::chrono::steady_clock::now() });
}

void CProfiler::End_CPU()
{
    if (!m_bInFrame || m_CPUStack.empty())
        return;

    const CPU_OPEN tOpen = m_CPUStack.back();
    m_CPUStack.pop_back();

    const _double dMS = std::chrono::duration<_double, std::milli>(std::chrono::steady_clock::now() - tOpen.t0).count();
    m_CPUSamples.push_back({ tOpen.szName, dMS });
}

void CProfiler::Set_Count(const _char* szName, _double dValue)
{
    if (m_bInFrame)
        m_Counts.push_back({ szName, dValue });
}

void CProfiler::Begin_GPU(const _char* szName)
{
    if (!m_pCur || m_pCur->iCount >= MAX_GPU_SCOPES)
    {
        m_GPUStack.push_back(INVALID_SCOPE);
        return;
    }

    const _uint iScope = m_pCur->iCount++;
    m_pCur->szNames[iScope] = szName;
    m_pContext->End(m_pCur->pBegin[iScope].Get());
    m_GPUStack.push_back(iScope);
}

void CProfiler::End_GPU()
{
    if (m_GPUStack.empty())
        return;

    const _uint iScope = m_GPUStack.back();
    m_GPUStack.pop_back();

    if (!m_pCur || iScope == INVALID_SCOPE)
        return;

    m_pContext->End(m_pCur->pEnd[iScope].Get());
}

void CProfiler::Collect_GPU()
{
    /* 이번에 도착한 프레임이 없으면 비워 둔다 (같은 값을 두 번 집계하지 않도록) */
    m_GPUSamples.clear();

    /* 오래된 프레임부터. 앞 프레임이 안 끝났으면 뒤 프레임도 안 끝났다. */
    for (_uint k = 0; k < FRAME_LATENCY; ++k)
    {
        GPU_FRAME& tFrame = m_Frames[(m_iWrite + k) % FRAME_LATENCY];
        if (!tFrame.bPending)
            continue;

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT tDisjoint{};
        if (S_OK != m_pContext->GetData(tFrame.pDisjoint.Get(), &tDisjoint, sizeof(tDisjoint), D3D11_ASYNC_GETDATA_DONOTFLUSH))
            return;

        tFrame.bPending = false;

        if (tDisjoint.Disjoint || tDisjoint.Frequency == 0) /* 클럭이 바뀐 프레임: 값을 믿을 수 없음 */
            continue;

        m_GPUSamples.clear();
        for (_uint i = 0; i < tFrame.iCount; ++i)
        {
            UINT64 iBegin = 0, iEnd = 0;
            if (S_OK != m_pContext->GetData(tFrame.pBegin[i].Get(), &iBegin, sizeof(iBegin), D3D11_ASYNC_GETDATA_DONOTFLUSH) ||
                S_OK != m_pContext->GetData(tFrame.pEnd[i].Get(), &iEnd, sizeof(iEnd), D3D11_ASYNC_GETDATA_DONOTFLUSH))
                continue;

            m_GPUSamples.push_back({ tFrame.szNames[i], _double(iEnd - iBegin) * 1000.0 / _double(tDisjoint.Frequency) });
        }
    }
}

NS_END
