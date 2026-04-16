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
public :
    virtual ~CTitan() = default;
    virtual void On_Grab(SIDE eSide, CHuman* pHuman) = 0;
    virtual void On_Dead(const _float fAccuracy) {};
    virtual void On_Stunned() {};

    _bool           Is_Alive() const {
        return m_bAlive;
    }
    void            Set_Dead() {
        m_bAlive = false;
    }

    virtual _bool   Is_Moving() = 0;
private :
    _bool   m_bAlive = true;
};



NS_END
