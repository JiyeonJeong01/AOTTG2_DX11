#include "CRender_System.h"

#include "Event_System.h"
#include "WindowResize_Event.h"

IMPLEMENT_SINGLETON(CRender_System)

CRender_System::CRender_System()
{
}

CRender_System::~CRender_System()
{
}

HRESULT CRender_System::Initialize(_uint iWidth, _uint iHeight)
{
    m_gUI.vViewport = { SCAST(_float, iWidth), SCAST(_float, iHeight) };

    /* Register event */
    SYS_EVENT.Subscribe(EVENT_TYPE::On_Window_Resize, &CRender_System::On_Resize, this);

    return S_OK;
}

void CRender_System::Render()
{

}
void CRender_System::On_Resize(EVENT_DATA& eData)
{
    assert(eData.eType == EVENT_TYPE::On_Window_Resize);

    auto& eResizeData = SCAST(RESIZE_EVENT_DATA&, eData);
    m_gUI.vViewport = { SCAST(_float, eResizeData.iWidth), SCAST(_float, eResizeData.iHeight) };

}
