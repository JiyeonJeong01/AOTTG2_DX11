#include "Tester.h"
#include "Engine_Log.h"
#include "MainApp.h"

void Client::Tester::Initialize_Tester(CMainApp* pMainapp)
{
    LOG_INFO("========== Lambda Test ========");
    pMainapp->m_voidEvent.Add_Listener([&](){Test_Public_Func_void(); });
    pMainapp->m_intEvent.Add_Listener([&](_int i) {Test_Public_Func_int(i); });
    pMainapp->m_intFloatEvent.Add_Listener([&](_int i, _float f) {Test_Public_Func_float_int(i, f); });

    LOG_INFO("========== Member Test ========");
    pMainapp->m_voidEvent.Add_Listener(&Tester::Test_Private_Func_void, this);
    pMainapp->m_intEvent.Add_Listener(&Tester::Test_Private_Func_int, this);
    pMainapp->m_intFloatEvent.Add_Listener(&Tester::Test_Private_Func_int_float, this);
}

void Client::Tester::Test_Public_Func_void()
{
    LOG_INFO("void");
}

void Client::Tester::Test_Public_Func_int(_int iTest)
{
    LOG_INFO("%d", iTest);
}

void Client::Tester::Test_Public_Func_float_int(_int iTest, _float fTest)
{
    LOG_INFO("%d, %f", iTest, fTest);
}

void Client::Tester::Test_Private_Func_void()
{
    LOG_INFO("void");
}

void Client::Tester::Test_Private_Func_int(_int iTest)
{
    LOG_INFO("%d", iTest);
}

void Client::Tester::Test_Private_Func_int_float(_int iTest, _float fTest)
{
    LOG_INFO("%d, %f", iTest, fTest);
}

Client::Tester* Client::Tester::Create()
{
    return new Tester;
}

void Client::Tester::Free()
{
    CBase::Free();
}
