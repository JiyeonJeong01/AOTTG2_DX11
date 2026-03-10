#include "PrototypeTest.h"
#include "GameObject_System.h"

NS_BEGIN(Client)

void CPrototypeTest::Awake(void* pCtx)
{
}

void CPrototypeTest::Start(void* pCtx)
{
    Engine::CGameObject* pObject = SYS_GAMEOBJECT.Get_Wrapper(m_hObject);
    m_pTarget = SYS_GAMEOBJECT.Get_Wrapper(m_rObject.hObject);
}

void CPrototypeTest::Priority_Update(void* pCtx, _float fDT)
{
}

void CPrototypeTest::Update(void* pCtx, _float fDT)
{




}

void CPrototypeTest::Late_Update(void* pCtx, _float fDT)
{
}

NS_END;
