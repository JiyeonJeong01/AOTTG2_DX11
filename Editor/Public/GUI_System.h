#pragma once
#include "Editor_Define.h"

NS_BEGIN(Editor)

class CGUI_System
{
    DECLARE_SINGLETON(CGUI_System)

public:
    HRESULT Initialize();
    void    Begin_Render();
    void    End_Render();

    ID3D11Device*           m_pDevice{};
    ID3D11DeviceContext*    m_pContext{};
    ImGuiContext*           m_pGuiContext{};

private :
    void    Setup_ImGuiStyle();
    _bool   m_bImguiInited = false;

};

NS_END;
