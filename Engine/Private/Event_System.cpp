#include "Event_System.h"

IMPLEMENT_SINGLETON(CEvent_System)


CEvent_System::CEvent_System()
{
}

CEvent_System::~CEvent_System()
{
    m_Events.clear();
}

HRESULT CEvent_System::Initialize()
{

    return S_OK;
}
