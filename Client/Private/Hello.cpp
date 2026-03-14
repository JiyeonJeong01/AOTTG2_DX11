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
    m_Transform = pObject->Get_Component<CTransform>();


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
    if (!m_pTarget)
        return;

    m_Transform = m_pTarget->Get_Component<CTransform>();
    m_SpringJoint = m_pTarget->Get_Component<CSpringJoint>();


    _float fSpeed = (_float)m_iSpeed * fDT;

    if (SYS_INPUT.Get_Key(VK_UP))
    {
        m_Transform.Translate({ 0.f, 0.f , fSpeed });
    }
    if (SYS_INPUT.Get_Key(VK_DOWN))
    {
        m_Transform.Translate({ 0.f, 0.f , -fSpeed });
    }
    if (SYS_INPUT.Get_Key(VK_RIGHT))
    {
        m_Transform.Translate({ fSpeed, 0.f, 0.f });
    }
    if (SYS_INPUT.Get_Key(VK_LEFT))
    {
        m_Transform.Translate({ -fSpeed, 0.f,  0.f });
    }
    if (SYS_INPUT.Get_KeyDown(VK_LBUTTON))
    {
        _float3 vAnchor = { m_Transform->vPosition.x - 5.f, m_Transform->vPosition.y + 5.f, m_Transform->vPosition.z + 5.f };
        m_SpringJoint.Set_Anchor(vAnchor);
    }
    if (SYS_INPUT.Get_KeyDown(VK_RBUTTON))
    {
        _float3 vAnchor = { m_Transform->vPosition.x + 5.f, m_Transform->vPosition.y + 5.f, m_Transform->vPosition.z + 5.f };
        m_SpringJoint.Set_Anchor(vAnchor);
    }
}

void CHello::Late_Update(void* pCtx, _float fDT)
{
}

void CHello::Move(_float fDT)
{

}

NS_END;
