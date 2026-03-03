#include "FirstScene.h"

#include <Transform.h>

#include "Component_System.h"

namespace Engine
{
    class CTransform;
}

NS_BEGIN(Client)
    void CFirstScene::Awake(void* pCtx)
{
}

void CFirstScene::Start(void* pCtx)
{
}

void CFirstScene::Priority_Update(void* pCtx, _float fDT)
{
}

void CFirstScene::Update(void* pCtx, _float fDT)
{
    _fvector vDir = Engine::Math::Right_Vec();

    SYS_COMPONENT.Get_Proxy<CTransform>(COMPONENT_TYPE::TRANSFORM, COMPONENT_HANDLE{ 1 }).
        Translate(vDir * fDT);
}

void CFirstScene::Late_Update(void* pCtx, _float fDT)
{
}

NS_END;
