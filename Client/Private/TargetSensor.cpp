#include "TargetSensor.h"

#include "GameObject.h"
#include "Transform.h"
#include "Entity_Define.h"

NS_BEGIN(Client)

CTargetSensor::CTargetSensor()
{
}

CTargetSensor::~CTargetSensor()
{
}

void CTargetSensor::Awake(void* pCtx)
{
    m_goTitan = GAME_INSTANCE.Find_GameObject(m_refTitan.hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, , "m_goTitan is nullptr");

    CGameObject* pBound = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(pBound, , "m_goTitan is nullptr");

    m_trTitan = m_goTitan->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(!m_trTitan.Is_Valid(), , "m_trTitan is invalid");

    m_trSensor = pBound->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(!m_trSensor.Is_Valid(), , "m_trTitan is invalid");

    CCollider trigger = pBound->Get_Component<CCollider>();
    IF_TRUE_RETURN_MSG_BREAK(!m_trSensor.Is_Valid(), , "m_trTitan is invalid");

    trigger->OnTriggerEnter.Add_Listener(&CTargetSensor::OnTriggerEnter, this);
}

void CTargetSensor::Start(void* pCtx)
{
    IScript::Start(pCtx);
}

void CTargetSensor::Priority_Update(void* pCtx, _float fDT)
{
    /* 수동 업데이트 필요 */
    m_trSensor->vPosition = m_trTitan->vPosition;

    if (!m_goTarget)
    {
        m_tDisplacement = DISPLACEMENT(_float3{});
        return;
    }

    m_tDisplacement = Detect_Target(m_goTarget);
}

void CTargetSensor::Update(void* pCtx, _float fDT)
{
    IScript::Update(pCtx, fDT);
}

void CTargetSensor::Late_Update(void* pCtx, _float fDT)
{
    IScript::Late_Update(pCtx, fDT);
}

void CTargetSensor::Set_Target(Engine::CGameObject* goTarget)
{
    m_goTarget = goTarget;
    m_tDisplacement = Detect_Target(goTarget);
}

void CTargetSensor::Clear_Target()
{
    m_goTarget = nullptr;
    m_tDisplacement = DISPLACEMENT(_float3{});
}

_bool CTargetSensor::Has_Target() const
{
    return m_goTarget != nullptr;
}

Engine::CGameObject* CTargetSensor::Get_Target() const
{
    return m_goTarget;
}

DISPLACEMENT CTargetSensor::Detect_Target(Engine::CGameObject* goTarget)
{
    if (!m_goTitan || !m_trTitan.Is_Valid() || !goTarget)
        return DISPLACEMENT(_float3{});

    CTransform trTarget = goTarget->Get_Component<CTransform>();
    if (!trTarget.Is_Valid())
        return DISPLACEMENT(_float3{});

    const _float3 vTitanPos = m_trTitan->vPosition;
    const _float3 vTargetPos = trTarget->vPosition;

    _float3 vToTarget{};
    vToTarget.x = vTargetPos.x - vTitanPos.x;
    vToTarget.y = vTargetPos.y - vTitanPos.y;
    vToTarget.z = vTargetPos.z - vTitanPos.z;

    return DISPLACEMENT(vToTarget);
}

DISPLACEMENT CTargetSensor::Get_TargetDisplacement() const
{
    return m_tDisplacement;
}

void CTargetSensor::OnTriggerEnter(const COLLISION_DESC& tDesc)
{
    CGameObject* pObject = GAME_INSTANCE.Find_GameObject(tDesc.hObject);
    if (!pObject)
        return;

    if (pObject->Has_Mask(O_HUMAN))
    {
        m_OnDetected_Human.Invoke(pObject);
    }
}

NS_END
