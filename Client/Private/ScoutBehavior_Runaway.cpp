#include "ScoutBehavior_Runaway.h"

CScoutBehavior_Runaway::CScoutBehavior_Runaway(CGameObject* goScout, CScout* scScout, SCOUT_BEHAVIOR eBehavior)
    : CScoutBehavior(goScout, scScout, eBehavior)
{
}

CScoutBehavior_Runaway::~CScoutBehavior_Runaway()
{
}

void CScoutBehavior_Runaway::Initialize()
{
    CScoutBehavior::Initialize();
}

void CScoutBehavior_Runaway::Priority_Update(_float fDT)
{
    CScoutBehavior::Priority_Update(fDT);
}

void CScoutBehavior_Runaway::Update(_float fDT)
{
    CScoutBehavior::Update(fDT);
}

void CScoutBehavior_Runaway::Late_Update(_float fDT)
{
    CScoutBehavior::Late_Update(fDT);
}

void CScoutBehavior_Runaway::Runaway(_float fDT)
{
    const _float fMaxSpeed = m_pStats->fMaxSpeed;

    _vector vCurPos = XMLoadFloat3(&m_tComponents.transform->vPosition);
    _vector vDir = XMLoadFloat3(&m_vTargetPos) - vCurPos;
    vDir = XMVectorSetY(vDir, 0.f);
    vDir = XMVector3Normalize(vDir);

    const _float fMoveLenSq = XMVectorGetX(XMVector3LengthSq(vDir));

    /* 입력 없음 */
    if (fMoveLenSq <= 0.f)
        return;

    vDir *= fMaxSpeed;
    m_tComponents.transform.Translate(vDir, SPACE::WORLD);
}

void CScoutBehavior_Runaway::Set_TargetPos(_fvector vPosition)
{
    XMStoreFloat3(&m_vTargetPos, vPosition);
}

std::shared_ptr<CScoutBehavior_Runaway> CScoutBehavior_Runaway::Create(Engine::CGameObject* goPlayer, CScout* scScout, SCOUT_BEHAVIOR eBehaviour)
{
    auto pInstance = std::make_shared<CScoutBehavior_Runaway>(goPlayer, scScout, eBehaviour);
    return pInstance;
}
