#pragma once

#include "Client_Define.h"

NS_BEGIN(Client)
class CTitan;

class CHuman
{
public:
    virtual ~CHuman() = default;
    virtual void On_Grabbed(SIDE eSide, CTitan* pTitan) = 0;
    virtual void On_Dead() {};

protected :
    ENTITY_VOLUME   tVolume{};
};

class CTitan
{
public:
    virtual ~CTitan() = default;
    virtual void On_Grab(SIDE eSide, CHuman* pHuman) = 0;
    virtual void On_Dead(const _float fAccuracy) {};

};



NS_END
