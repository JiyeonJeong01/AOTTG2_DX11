#include "Tester.h"
#include "Engine_Log.h"
#include "MainApp.h"
#include "Event.h"

NS_BEGIN(Client)

void Tester::Initialize_Tester(CMainApp* pMainapp)
{
}

void Tester::Test_Public_Func_void()
{
}

void Tester::Test_Public_Func_int(_int iTest)
{
}

void Tester::Test_Public_Func_float_int(_int iTest, _float fTest)
{
}

void Tester::Test_Private_Func_void()
{
}

void Tester::Test_Private_Func_int(_int iTest)
{
}

void Client::Tester::Test_Private_Func_int_float(_int iTest, _float fTest)
{
}

Tester* Client::Tester::Create()
{
    return new Tester;
}

void Tester::Free()
{
    CBase::Free();
}

NS_END
