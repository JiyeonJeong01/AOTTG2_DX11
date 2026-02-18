#include "CRender_System.h"

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

    return S_OK;
}

void CRender_System::Render()
{

}

void CRender_System::On_Resize(_uint iWidth, _uint iHeight)
{
    m_gUI.vViewport = { SCAST(_float, iWidth), SCAST(_float, iHeight) };

}
