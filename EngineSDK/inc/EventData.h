#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CEventData : public CBase
{
public:
    CEventData(EVENT_TYPE eType) : m_eType(eType) {}
    virtual ~CEventData() = default;

    EVENT_TYPE GetType() const { return m_eType; }

private:
    EVENT_TYPE m_eType;
};

NS_END
