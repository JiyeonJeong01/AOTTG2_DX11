#pragma once

#include "Client_Define.h"

NS_BEGIN(Client)

class CHuman
{
public:
    virtual ~CHuman() = default;

protected :
    ENTITY_VOLUME   tVolume{};
};

class CTitan
{
public:
    virtual ~CTitan() = default;
};



NS_END
