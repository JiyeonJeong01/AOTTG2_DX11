#pragma once
#include "Base.h"

NS_BEGIN(Client)

class Tester : public CBase
{
public:
    void Initialize_Tester(class CMainApp* pMainapp);

public :
    void Test_Public_Func_void();
    void Test_Public_Func_int(_int iTest);
    void Test_Public_Func_float_int(_int iTest, _float fTest);


private :
    void Test_Private_Func_void();
    void Test_Private_Func_int(_int iTest);
    void Test_Private_Func_int_float(_int iTest, _float fTest);

private :
    int iMemTest{};
    float fMemTest{};

public:
    static Tester* Create();
    virtual void Free() override;
};

NS_END

