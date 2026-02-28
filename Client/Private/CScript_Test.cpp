#include "CScript_Test.h"
#include "Input_System.h"
#include "Component_System.h"

NS_BEGIN(Client)

void CScript_Test::Awake(void* pCtx)
{
    IScript::Awake(pCtx);
}

void CScript_Test::Start(void* pCtx)
{
    IScript::Start(pCtx);

    m_transform = SYS_COMPONENT.Get_Proxy<CTransform>(COMPONENT_TYPE::TRANSFORM, COMPONENT_HANDLE{ 1 });
    __noop;
}

void CScript_Test::Priority_Update(void* pCtx, _float fDT)
{
}

void CScript_Test::Update(void* pCtx, _float fDT)
{
    Handle_Input(fDT);
}

void CScript_Test::Late_Update(void* pCtx, _float fDT)
{
}

void CScript_Test::Handle_Input(_float fDT)
{
    if (SYS_INPUT.Get_Key('W'))
    {
        m_transform.Translate({ 0.f, 0.f, 1.f * fDT, 0.f });
    }
    if (SYS_INPUT.Get_Key('A'))
    {
        m_transform.Translate({ -1.f * fDT, 0.f, 0.f, 0.f });
    }
    if (SYS_INPUT.Get_Key('S'))
    {
        m_transform.Translate({ 0.f, 0.f, -1.f * fDT, 0.f });
    }
    if (SYS_INPUT.Get_Key('D'))
    {
        m_transform.Translate({ 1.f * fDT, 0.f, 0.f, 0.f });
    }

}

NS_END
