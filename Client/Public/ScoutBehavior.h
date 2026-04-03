#pragma once
#include "Scout_Define.h"

NS_BEGIN(Client)

class CScout;

class CScoutBehavior
{
public :
    CScoutBehavior(Engine::CGameObject* goScout, CScout* scScout, SCOUT_BEHAVIOR eBehavior);
    virtual ~CScoutBehavior();

public :
    virtual void Initialize();
    virtual void Priority_Update(_float fDT);
    virtual void Update(_float fDT);
    virtual void Late_Update(_float fDT);

    void Cache_ScoutContext(const SCOUT_CONTEXT& tContext);
    virtual void SetUp_CachedScoutContext();

protected:
    CGameObject*        m_goScout{};
    CScout*             m_scScout{};

    SCOUT_COMPONENTS    m_tComponents{};
    SCOUT_STATS*        m_pStats{};

    SCOUT_BEHAVIOR m_eBehaviour = SCOUT_BEHAVIOR::END;
};

NS_END
