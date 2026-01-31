#pragma once
#include "Base.h"
#include "Editor_Define.h"

NS_BEGIN(Editor)

class CGUI_System : public CBase
{
    DECLARE_SINGLETON(CGUI_System)
private:
    CGUI_System();
    ~CGUI_System() override = default;

public:
    HRESULT Ready_System();
    void    Update();
    void    Render_GUI();

    LRESULT         Engine_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    ImGuiContext* Share_GuiContext() { return m_pGuiContext; }

private:
    ID3D11Device* m_pDevice{};
    ID3D11DeviceContext* m_pContext{};
    ImGuiContext* m_pGuiContext{};

public:
    static CGUI_System* Create(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
private:
    void Free() override;
};

NS_END;
