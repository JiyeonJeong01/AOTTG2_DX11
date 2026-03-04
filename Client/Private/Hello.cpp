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
}

void CHello::Priority_Update(void* pCtx, _float fDT)
{
}

void CHello::Update(void* pCtx, _float fDT)
{
    _vector vDir = Engine::Math::Right_Vec();
    m_Trnasform.Translate(vDir * fDT);
}

void CHello::Late_Update(void* pCtx, _float fDT)
{
}

void CHello::Move(_float fDT)
{

}

NS_END;
