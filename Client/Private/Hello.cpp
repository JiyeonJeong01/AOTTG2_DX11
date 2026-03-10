#include "Hello.h"
#include "GameObject.h"
#include "GameObject_System.h"
#include "Transform.h"
#include "Input_System.h"
#include "Rigidbody.h"
#include "Logger.h"

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
    if (nullptr == m_pTarget && m_rObject.hObject.Is_Valid())
    {
       m_pTarget = SYS_GAMEOBJECT.Get_Wrapper(m_rObject.hObject);
    }

    auto rb = m_pTarget->Get_Component<CRigidbody>();
    if (SYS_INPUT.Get_KeyDown('P'))
    {
        rb.Add_LinearImpulse({ 0.f, (_float)m_iSpeed, 0.f });
    }

    _float fSpeed = (_float)m_iSpeed * fDT;

    if (SYS_INPUT.Get_Key(VK_UP))
    {
        rb.Translate({ 0.f, 0.f , fSpeed });
    }
    if (SYS_INPUT.Get_Key(VK_DOWN))
    {
        rb.Translate({ 0.f, 0.f , -fSpeed });
    }
    if (SYS_INPUT.Get_Key(VK_RIGHT))
    {
        rb.Translate({ fSpeed, 0.f, 0.f });
    }
    if (SYS_INPUT.Get_Key(VK_LEFT))
    {
        rb.Translate({ -fSpeed, 0.f,  0.f });
    }

    if (rb._Data())
    {
        LOG_INFO("linear velocity : %.1f, %.1f, %.1f", rb->vLinearVel.x, rb->vLinearVel.y, rb->vLinearVel.z);
        LOG_INFO("Force Accum : %.1f, %.1f, %.1f", rb->vForceAccum.x, rb->vForceAccum.y, rb->vForceAccum.z);
    }

}

void CHello::Late_Update(void* pCtx, _float fDT)
{
}

void CHello::Move(_float fDT)
{

}

NS_END;
