#pragma once

#include "Client_Define.h"

NS_BEGIN(Client)

class CState abstract 
{
public :
    CState() = default;
    ~CState() = default;

    virtual void Enter(_uint iDetailFlag) = 0;
    virtual void Exit() = 0;
};

NS_END
