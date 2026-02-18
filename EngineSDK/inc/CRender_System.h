#pragma once
#include "Engine_Define.h"
#include "Render_Struct.h"

NS_BEGIN(Engine)

class CRender_System final
{
    DECLARE_SINGLETON(CRender_System)

public :
    HRESULT Initialize(_uint iWidth, _uint iHeight);
    
    void    Render();

    void    On_Resize(_uint iWidth, _uint iHeight);
    const UI_GLOBAL& Get_UI_Global() { return m_gUI; }

private :
    UI_GLOBAL   m_gUI{};
};

NS_END
