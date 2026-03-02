#include "Test_ComponentEnable.h"
#include "GameObject_System.h"
#include "GameObject.h"
#include "Transform.h"

NS_BEGIN(Client)
    void CTest_ComponentEnable::Awake(void* pCtx)
{
}

void CTest_ComponentEnable::Start(void* pCtx)
{

}

void CTest_ComponentEnable::Priority_Update(void* pCtx, _float fDT)
{
}

void CTest_ComponentEnable::Update(void* pCtx, _float fDT)
{
    //OBJECT_HANDLE hObj;
    //hObj.raw = 65538;
    //SYS_GAMEOBJECT.Get_Wrapper(hObj)->Get_Component<CTransform>(COMPONENT_TYPE::TRANSFORM).Translate(Engine::Math::Look_Vec() * fDT);
}

void CTest_ComponentEnable::Late_Update(void* pCtx, _float fDT)
{
}

NS_END;
