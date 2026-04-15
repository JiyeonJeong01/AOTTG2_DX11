#include "Scout_Scriptable_Object.h"

NS_BEGIN(Client)

CScout_Scriptable_Object::CScout_Scriptable_Object()
{
}

CScout_Scriptable_Object::~CScout_Scriptable_Object()
{
}

void CScout_Scriptable_Object::Awake(void* pCtx)
{
}

void CScout_Scriptable_Object::Start(void* pCtx)
{
}

void CScout_Scriptable_Object::Priority_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(fDT);
}

void CScout_Scriptable_Object::Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(fDT);
}

void CScout_Scriptable_Object::Late_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(fDT);
}

SCOUT_BEHAVIOR CScout_Scriptable_Object::Get_Behavior() const
{
    return Convert_Behavior();
}

SCOUT_BEHAVIOR CScout_Scriptable_Object::Convert_Behavior() const
{
    if (0 == lstrcmpiA(m_szBehaviorName, "REQUEST_RESUPPLY"))
        return SCOUT_BEHAVIOR::REQUEST_RESUPPLY;

    if (0 == lstrcmpiA(m_szBehaviorName, "NONE"))
        return SCOUT_BEHAVIOR::NONE;

    return SCOUT_BEHAVIOR::NONE;
}

NS_END
