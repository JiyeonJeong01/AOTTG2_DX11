#pragma once

#include "State.h"

NS_BEGIN(Client)

class CStateMachine abstract
{
public :
    CStateMachine() = default;
    ~CStateMachine() = default;

public :
    virtual void Change_State(_uint iStateKey, _uint iDetailFlag) = 0;

protected :
    _uint                   m_iCurStateKey{};
    _uint                   m_iPrevStateKey{};

};

NS_END
