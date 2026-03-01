#pragma once

#include "Engine_Define.h"

NS_BEGIN(Engine)

class ENGINE_DLL CInput_System final
{
    DECLARE_SINGLETON(CInput_System)
public:
    HRESULT	Initialize(HWND hWnd, HINSTANCE hInst);
    void	Update_System();

    _bool	Get_Key(int iKey);
    _bool	Get_KeyDown(int iKey);
    _bool	Get_KeyUp(int iKey);

    _byte	Get_DIMouseState(MOUSE_BUTTON eMouse) const
    {
        return m_tMouseState.rgbButtons[SCAST(_uint, eMouse)];
    }

    long Get_DIMouseMove(MOUSE_MOVE_AXIS eMouseState)
    {
        switch (eMouseState)
        {
        case MOUSE_MOVE_AXIS::HORIZONTAL: return m_tMouseState.lX;
        case MOUSE_MOVE_AXIS::VERTICAL:   return m_tMouseState.lY;
        case MOUSE_MOVE_AXIS::DEPTH:      return m_tMouseState.lZ;
        default:                          return 0;
        }
    }

    const POINT& Get_MousePos() const;

private:
    static constexpr int	KEY_CNT = 0xff;
    _bool					m_bPrevPress[KEY_CNT];
    _bool					m_bCurPress[KEY_CNT];

    LPDIRECTINPUT8			m_pInputSDK = nullptr;
    LPDIRECTINPUTDEVICE8	m_pMouse = nullptr;
    DIMOUSESTATE			m_tMouseState;

    HWND                    m_hWnd{};
    POINT                   m_tMousePos{};
};

NS_END
