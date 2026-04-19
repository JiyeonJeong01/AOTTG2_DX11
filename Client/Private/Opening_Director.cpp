#include "Opening_Director.h"
#include "GameInstance.h"

NS_BEGIN(Client)

void COpening_Director::Awake(void* pCtx)
{
    CGameObject* pObject = GAME_INSTANCE.Find_GameObject(m_refBoat.hObject);

    if (pObject != nullptr)
    {
        m_pBoatObject = pObject;
        m_trBoat = m_pBoatObject->Get_Component<CTransform>();
    }
}

void COpening_Director::Start(void* pCtx)
{
    if (!m_trBoat.Is_Valid())
        return;

    m_vBoatStartPos = m_trBoat->vPosition;
    m_vBoatBasePos = m_vBoatStartPos;
    m_vBoatBaseEuler = m_trBoat.Get_Rotation_Euler();

    Enter_State(OPENING_STATE::BOAT_APPROACH);
}

void COpening_Director::Priority_Update(void* pCtx, _float fDT)
{
}

void COpening_Director::Update(void* pCtx, _float fDT)
{
    if (!m_trBoat.Is_Valid())
        return;

    m_fOpeningTime += fDT;
    m_fStateTime += fDT;

    switch (m_eState)
    {
    case OPENING_STATE::BOAT_APPROACH:
        Update_BoatApproach(fDT);
        break;

    case OPENING_STATE::BOAT_ARRIVED_WAIT:
        if (m_fStateTime >= 1.0f)
            Enter_State(OPENING_STATE::WHITEOUT);
        break;

    case OPENING_STATE::WHITEOUT:
        /* 나중에 UI 화이트 패널 alpha 증가 */
        if (m_fStateTime >= 1.0f)
            Enter_State(OPENING_STATE::TITAN_REVEAL);
        break;

    case OPENING_STATE::TITAN_REVEAL:
        /* 나중에 거인 등장 + 카메라 전환 */
        break;
    }
}

void COpening_Director::Late_Update(void* pCtx, _float fDT)
{
}

void COpening_Director::Enter_State(OPENING_STATE eState)
{
    m_eState = eState;
    m_fStateTime = 0.f;

    switch (m_eState)
    {
    case OPENING_STATE::BOAT_APPROACH:
        break;

    case OPENING_STATE::BOAT_ARRIVED_WAIT:
        break;

    case OPENING_STATE::WHITEOUT:
        break;

    case OPENING_STATE::TITAN_REVEAL:
        break;
    }
}

void COpening_Director::Update_BoatApproach(_float fDT)
{
    _float3 vCurBasePos = m_vBoatBasePos;

    _float3 vToGoal =
    {
        m_vBoatGoalPos.x - vCurBasePos.x,
        m_vBoatGoalPos.y - vCurBasePos.y,
        m_vBoatGoalPos.z - vCurBasePos.z
    };

    const _float fDistSq =
        vToGoal.x * vToGoal.x +
        vToGoal.y * vToGoal.y +
        vToGoal.z * vToGoal.z;

    /* 도착 */
    if (fDistSq <= (m_fBoatArriveDist * m_fBoatArriveDist))
    {
        m_vBoatBasePos = m_vBoatGoalPos;

        m_trBoat->vPosition = m_vBoatGoalPos;
        Enter_State(OPENING_STATE::BOAT_ARRIVED_WAIT);
        return;
    }

    const _float fDist = sqrtf(fDistSq);

    _float3 vDir =
    {
        vToGoal.x / fDist,
        vToGoal.y / fDist,
        vToGoal.z / fDist
    };

    /* 일단 이동 */
    m_vBoatBasePos.x += vDir.x * m_fBoatMoveSpeed * fDT;
    m_vBoatBasePos.y += vDir.y * m_fBoatMoveSpeed * fDT;
    m_vBoatBasePos.z += vDir.z * m_fBoatMoveSpeed * fDT;

    /* 꿀렁이는 효과 넣기 */
    _float3 vFinalPos = m_vBoatBasePos;
    vFinalPos.y += sinf(m_fOpeningTime * m_fBoatBobFreq) * m_fBoatBobAmp;

    m_trBoat.Set_Position(XMLoadFloat3(&vFinalPos));

    _float4 vRot = m_trBoat->vRotationQuat;

    /* 값 : 삼각함수 (시간 * 주파수) * 세기 */
    _float3 vEuler = m_vBoatBaseEuler;
    vEuler.x += cosf(m_fOpeningTime * m_fBoatPitchFreq) * m_fBoatBobAmp;
    vEuler.z += sinf(m_fOpeningTime * m_fBoatRollFreq) * m_fBoatRollAmp;

    m_trBoat.Set_Rotation_Euler(vEuler);
}

NS_END
