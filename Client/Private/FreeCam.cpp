#include "FreeCam.h"
#include "Input_System.h"
#include "GameObject.h"

NS_BEGIN(Client)

CGameObject* CFreeCam::Find_FreeCam()
{
    if(m_roCamera.Is_Valid())
    {
        m_goFreeCam = SYS_GAMEOBJECT.Get_Wrapper(m_roCamera.hObject);
    }

    if (m_goFreeCam)
    {
        m_Transform = m_goFreeCam->Get_Component<CTransform>();
    }

    return m_goFreeCam;
}

void CFreeCam::Awake(void* pCtx)
{
}

void CFreeCam::Start(void* pCtx)
{

}

void CFreeCam::Priority_Update(void* pCtx, _float fDT)
{
}

void CFreeCam::Update(void* pCtx, _float fDT)
{
}

void CFreeCam::Late_Update(void* pCtx, _float fDT)
{
    if (nullptr == m_goFreeCam)
    {
        Find_FreeCam();
        return;
    }
  
    _vector vDir{};
    
    if (SYS_INPUT.Get_Key('W'))
        vDir = XMVectorSetZ(vDir, -1.f);
    else if (SYS_INPUT.Get_Key('S'))
        vDir = XMVectorSetZ(vDir, 1.f);
    if (SYS_INPUT.Get_Key('A'))
        vDir = XMVectorSetX(vDir, -1.f);
    else if (SYS_INPUT.Get_Key('D'))
        vDir = XMVectorSetX(vDir, 1.f);

    vDir = XMVector4Normalize(vDir);

    m_Transform.Translate(vDir * m_fSpeed * fDT);

    long iMove = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::HORIZONTAL);
    if (iMove != 0)
        m_Transform.Rotate({ 0.f, 1.f, 0.f, 0.f }, iMove * m_fMouseSense * fDT, SPACE::WORLD);

    iMove = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::VERTICAL);
    if (iMove != 0)
        m_Transform.Rotate({ 1.f, 0.f, 0.f, 0.f }, iMove * m_fMouseSense * fDT, SPACE::LOCAL);
}

NS_END;
