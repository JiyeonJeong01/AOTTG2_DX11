#pragma once
#include "Editor_Define.h"

NS_BEGIN(Editor)

class CGUI_System
{
    DECLARE_SINGLETON(CGUI_System)

public:
    HRESULT Initialize();
    void    Update();
    void    Render_GUI();

    LRESULT         Engine_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    ImGuiContext* Share_GuiContext() { return m_pGuiContext; }

private:
    ID3D11Device* m_pDevice{};
    ID3D11DeviceContext* m_pContext{};
    ImGuiContext* m_pGuiContext{};

private :
    void    Setup_ImGuiStyle();

public:
    static CGUI_System* Create(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
};

NS_END;
