#pragma once
#include "ScoutBehavior.h"

NS_BEGIN(Client)

class CScout_Scriptable_Object final : public IScript
{
public:
    CScout_Scriptable_Object();
    ~CScout_Scriptable_Object() override;

public:

public:
    SCOUT_BEHAVIOR Get_Behavior() const;

private:
    SCOUT_BEHAVIOR Convert_Behavior() const;

private:
    _char m_szBehaviorName[64] = "NONE";

    SCRIPT_FIELDS_BEGIN(CScout_Scriptable_Object)
        SCRIPT_FIELD_CHAR(m_szBehaviorName);
    SCRIPT_FIELDS_END(CScout_Scriptable_Object)
};

NS_END
