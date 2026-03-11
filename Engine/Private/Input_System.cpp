#include "Input_System.h"

#include "Engine_Log.h"
#include "CRender_System.h"

IMPLEMENT_SINGLETON(CInput_System)

CInput_System::CInput_System()
{
    fill(m_bPrevPress, m_bPrevPress + KEY_CNT, false);
    fill(m_bCurPress, m_bCurPress + KEY_CNT, false);
}

CInput_System::~CInput_System()
{
    Safe_Release(m_pMouse);
    Safe_Release(m_pInputSDK);
}

HRESULT CInput_System::Initialize(HWND hWnd, HINSTANCE hInst)
{
    HRESULT hr;
    if (FAILED(hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pInputSDK, NULL)))
    {
        _DEBUG_ERROR("DirectInput8Create failed. hr=0x%08X", (uint32_t)hr);
        return E_FAIL;

    }

    if (FAILED(m_pInputSDK->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr)))
        return E_FAIL;

    m_pMouse->SetDataFormat(&c_dfDIMouse);
    m_pMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
    m_pMouse->Acquire();

    m_hWnd = hWnd;

    return S_OK;

}

void CInput_System::Update_System()
{
    for (int i = 0; i < KEY_CNT; ++i)
    {
        m_bPrevPress[i] = m_bCurPress[i];
    }

    fill(m_bCurPress, m_bCurPress + 0xfe, false);

    for (int i = 0; i < KEY_CNT; ++i)
    {
        if ((GetAsyncKeyState(i) & 0x8000) != 0)
            m_bCurPress[i] = true;
    }

    POINT ptScreen{};
    GetCursorPos(&ptScreen);

    POINT ptClient = ptScreen;
    ScreenToClient(m_hWnd, &ptClient);
    memcpy(&m_tMousePos, &ptClient, sizeof(POINT));

    /* SceneView 기준 게임 좌표 계산 */
    m_tGameMousePos.x = -1;
    m_tGameMousePos.y = -1;

    const auto ui = SYS_RENDER.Get_UI_Global();

    if (ui.tSceneView.vSize.x > 0.f && ui.tSceneView.vSize.y > 0.f)
    {
        const _float fLocalX = (_float)ptScreen.x - ui.tSceneView.vScreenPos.x;
        const _float fLocalY = (_float)ptScreen.y - ui.tSceneView.vScreenPos.y;

        if (fLocalX >= 0.f && fLocalY >= 0.f &&
            fLocalX < ui.tSceneView.vSize.x && fLocalY < ui.tSceneView.vSize.y)
        {
            const _float fGameX = fLocalX * (ui.vViewport.x / ui.tSceneView.vSize.x);
            const _float fGameY = fLocalY * (ui.vViewport.y / ui.tSceneView.vSize.y);

            m_tGameMousePos.x = SCAST(LONG, fGameX);
            m_tGameMousePos.y = SCAST(LONG, fGameY);
        }
    }

    m_pMouse->GetDeviceState(sizeof(m_tMouseState), &m_tMouseState);
}

_bool CInput_System::Get_Key(int iKey)
{
    if (iKey < 0 || iKey >= KEY_CNT) return false;
    return m_bCurPress[iKey];
}

_bool CInput_System::Get_KeyDown(int iKey)
{
    if (iKey < 0 || iKey >= KEY_CNT) return false;
    return (m_bCurPress[iKey] && !m_bPrevPress[iKey]);
}

_bool CInput_System::Get_KeyUp(int iKey)
{
    if (iKey < 0 || iKey >= KEY_CNT) return false;
    return (!m_bCurPress[iKey] && m_bPrevPress[iKey]);
}

const POINT& CInput_System::Get_MousePos() const
{
    return m_tMousePos;
}

const POINT& CInput_System::Get_GameMousePos() const
{
    return m_tGameMousePos;
}
