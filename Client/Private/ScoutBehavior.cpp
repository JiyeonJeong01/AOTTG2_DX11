#include "ScoutBehavior.h"


CScoutBehavior::CScoutBehavior(Engine::CGameObject* goScout, CScout* scScout, SCOUT_BEHAVIOR eBehavior)
    : m_goScout(goScout), m_scScout(scScout), m_eBehaviour(eBehavior)
{
}

CScoutBehavior::~CScoutBehavior()
{
}

void CScoutBehavior::Initialize()
{
}

void CScoutBehavior::Priority_Update(_float fDT)
{
}

void CScoutBehavior::Update(_float fDT)
{
}

void CScoutBehavior::Late_Update(_float fDT)
{
}

void CScoutBehavior::Cache_ScoutContext(const SCOUT_CONTEXT& tContext)
{
    m_tComponents = tContext.tComponents;
    m_pStats = tContext.pStat;
}

void CScoutBehavior::SetUp_CachedScoutContext()
{

}
