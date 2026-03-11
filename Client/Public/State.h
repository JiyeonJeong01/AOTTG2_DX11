#pragma once

#include "Client_Define.h"

NS_BEGIN(Client)

class CState abstract 
{
public :
    CState() = default;
    ~CState() = default;

    virtual void Enter() = 0;
};

NS_END
