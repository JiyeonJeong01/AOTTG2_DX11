#include "Hello.h"
#include "GameObject.h"
#include "GameObject_System.h"
#include "Transform.h"

NS_BEGIN(Client)

void CHello::Awake(void* pCtx)
{
}

void CHello::Start(void* pCtx)
{
    Engine::CGameObject* pObject = SYS_GAMEOBJECT.Get_Wrapper(m_hObject);
    m_Trnasform = pObject->Get_Component<CTransform>();


    m_pTarget = SYS_GAMEOBJECT.Get_Wrapper(m_rObject.hObject);
}

void CHello::Priority_Update(void* pCtx, _float fDT)
{
}

void CHello::Update(void* pCtx, _float fDT)
{
    if (m_pTarget)
    {
        m_pTarget->Get_Component<CTransform>().Translate(Math::Load(m_vDir) * m_iSpeed * fDT);
        LOG_INFO("m_pTarget is moving...");
    }
}

void CHello::Late_Update(void* pCtx, _float fDT)
{
}

void CHello::Move(_float fDT)
{

}

NS_END;
