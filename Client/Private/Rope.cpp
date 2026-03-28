#include "Rope.h"
#include "GameInstance.h"

NS_BEGIN(Client)

CRope::CRope(SIDE eSide)
    : m_eSide(eSide)
{
}

CRope::~CRope()
{
}

void CRope::Initialize()
{
    m_upLine = GAME_INSTANCE.Load_LineMesh(m_iNumPoints, 0.3f);
    m_RopePoints.reserve(m_iNumPoints);

    m_tDynamicValue.fGain = 18.f;
    m_tDynamicValue.fDamping = 8.f;
    m_tDynamicValue.fTarget = 1.f;
    m_tDynamicValue.fVelocity = 0.f;
    m_tDynamicValue.fValue = 0.f;
}

void CRope::Update(_float fTimeDelta)
{
    switch (m_State)
    {
    case ROPE_STATE::IDLE:
        m_RopePoints.clear();
        break;

    case ROPE_STATE::EXTENDING_SUCCESS: case ROPE_STATE::EXTENDING_FAIL :
        Process_Extending(fTimeDelta);
        break;

    case ROPE_STATE::ANCHORED:
        Process_Anchored();
        break;

    case ROPE_STATE::RETURNING:
        Process_Retuning(fTimeDelta);
        break;
    }

    Upload_Line();
}

void CRope::Render()
{
    if (!m_upLine)
        return;

    if (m_State == ROPE_STATE::IDLE)
        return;

    if (m_RopePoints.size() < 2)
        return;

    m_upLine->Submit();
}

void CRope::Start_Extending_Success(_fvector vStartPoint, _fvector vAnchorPoint)
{
    XMStoreFloat3(&m_vStartPoint, vStartPoint);
    XMStoreFloat3(&m_vCurDynamicPos, vStartPoint);
    XMStoreFloat3(&m_vEndPoint, vAnchorPoint);

    _vector vDir = vAnchorPoint - vStartPoint;
    if (XMVectorGetX(XMVector3LengthSq(vDir)) <= 1e-6f)
        vDir = Math::Set_Vec(0.f, 0.f, 1.f, 0.f);

    XMStoreFloat3(&m_vTrialDir, XMVector4Normalize(vDir));

    m_tDynamicValue.Reset();
    m_tDynamicValue.fTarget = 1.f;

    m_State = ROPE_STATE::EXTENDING_SUCCESS;
}

void CRope::Start_Extending_Fail(_fvector vStartPoint, _fvector vRopeDir)
{
    XMStoreFloat3(&m_vStartPoint, vStartPoint);
    XMStoreFloat3(&m_vCurDynamicPos, vStartPoint);
    XMStoreFloat3(&m_vEndPoint, vStartPoint + vRopeDir * m_fRopMaxLength);

    _vector vDir = XMLoadFloat3(&m_vEndPoint) - vStartPoint;
    if (XMVectorGetX(XMVector3LengthSq(vDir)) <= 1e-6f)
        vDir = Math::Set_Vec(0.f, 0.f, 1.f, 0.f);

    XMStoreFloat3(&m_vTrialDir, XMVector4Normalize(vDir));

    m_tDynamicValue.Reset();
    m_tDynamicValue.fTarget = 1.f;

    m_State = ROPE_STATE::EXTENDING_FAIL;
}

void CRope::Set_StartPoint(const _float3& vStartPoint)
{
    m_vStartPoint = vStartPoint;
}

void CRope::Set_Anchored(const _float3& vStartPoint, const _float3& vAnchorPoint)
{
    m_vStartPoint = vStartPoint;
    m_vEndPoint = vAnchorPoint;
    m_vCurDynamicPos = vAnchorPoint;

    m_State = ROPE_STATE::ANCHORED;
}

void CRope::Start_Returning(const _float3& vReturnTarget)
{
    const _vector vCur = Math::Load(m_vCurDynamicPos);   /* 지금 줄 끝 */ 
    const _vector vTarget = Math::Load(vReturnTarget);   /* 돌아갈 원래 시작점 */ 

    m_vEndPoint = vReturnTarget;

    _vector vDir = vTarget - vCur;
    if (Math::Get_X(XMVector3LengthSq(vDir)) <= 1e-6f)
        vDir = Math::Set_Vec(0.f, 0.f, 1.f, 0.f);

    XMStoreFloat3(&m_vTrialDir, Math::Normalize(vDir));

    m_tDynamicValue.Reset();
    m_tDynamicValue.fTarget = 1.f;

    m_State = ROPE_STATE::RETURNING;
}

void CRope::Stop()
{
    m_State = ROPE_STATE::IDLE;
    m_RopePoints.clear();
}

void CRope::Set_RopeAmplitueInfo(const AMPLITUDE_VALUE& tInfo)
{
    m_tDynamicValue = tInfo;
}

void CRope::Process_Extending(_float fTimeDelta)
{
    Calc_RopeShape(m_vStartPoint, fTimeDelta);

    const _vector vCurDynamicPos = Math::Load(m_vCurDynamicPos);
    const _vector vAnchor = Math::Load(m_vEndPoint);
    const _vector vTrialDir = Math::Load(m_vTrialDir);

    const _float fDist = Math::Get_X(XMVector3Length(vCurDynamicPos - vAnchor));
    const _vector vCurDir = vAnchor - vCurDynamicPos;
    const _float fDot = Math::Get_X(XMVector3Dot(vCurDir, vTrialDir));

    /* 목표 지점 도달 */
    if (fDist < 1.f || fDot < 0.f)
    {
        m_vCurDynamicPos = m_vEndPoint;
        if (m_State == ROPE_STATE::EXTENDING_SUCCESS)
        {
            m_State = ROPE_STATE::ANCHORED;
            m_OnChanged_RopeState.Invoke(ROPE_STATE::ANCHORED, m_eSide);
            return;
        }
        if (m_State == ROPE_STATE::EXTENDING_FAIL)
        {
            Start_Returning(m_vStartPoint);
            m_OnChanged_RopeState.Invoke(ROPE_STATE::RETURNING, m_eSide);
        }
    }
}

void CRope::Process_Anchored()
{
    m_RopePoints.clear();
    m_RopePoints.push_back(m_vStartPoint);
    m_RopePoints.push_back(m_vEndPoint);
}

void CRope::Process_Retuning(_float fTimeDelta)
{
    Calc_RopeShape(m_vStartPoint, fTimeDelta);

    if (Calc_RopeComplete(m_vStartPoint))
    {
        m_State = ROPE_STATE::IDLE;
        m_RopePoints.clear();

        m_OnChanged_RopeState.Invoke(ROPE_STATE::IDLE, m_eSide);
    }
}

void CRope::Calc_RopeShape(const _float3& vCurTip, _float fTimeDelta)
{
    _vector vCurDynamicPos = Math::Load(m_vCurDynamicPos);
    const _vector vTrialDir = Math::Load(m_vTrialDir);

    vCurDynamicPos += vTrialDir * m_fExtendVel * fTimeDelta;
    Math::Store(m_vCurDynamicPos, vCurDynamicPos);

    const _float fAmplitude = m_tDynamicValue.Get_Value(fTimeDelta);

    const _vector vTip = Math::Load(vCurTip);
    const _vector vAnchor = Math::Load(m_vEndPoint);

    _vector vDir = vAnchor - vTip;
    if (Math::Get_X(XMVector3LengthSq(vDir)) <= 1e-6f)
        vDir = Math::Set_Vec(0.f, 0.f, 1.f, 0.f);
    vDir = Math::Normalize(vDir);

    _vector vWorldUp = Math::Set_Vec(0.f, 1.f, 0.f, 0.f);
    _vector vRight = Math::Cross(vDir, vWorldUp);

    if (Math::Get_X(XMVector3LengthSq(vRight)) <= 1e-6f)
    {
        vWorldUp = Math::Set_Vec(0.f, 0.f, 1.f, 0.f);
        vRight = Math::Cross(vDir, vWorldUp);
    }

    if (Math::Get_X(XMVector3LengthSq(vRight)) <= 1e-6f)
        vRight = Math::Set_Vec(1.f, 0.f, 0.f, 0.f);

    vRight = Math::Normalize(vRight);

    m_RopePoints.clear();

    if (m_iNumPoints < 2)
        return;

    const _vector vDynamic = Math::Load(m_vCurDynamicPos);

    for (_int i = 0; i < m_iNumPoints; ++i)
    {
        const _float fRatio = static_cast<_float>(i) / static_cast<_float>(m_iNumPoints - 1);

        const _vector vBase = XMVectorLerp(vTip, vDynamic, fRatio);

        const _float fShape = sinf(fRatio * static_cast<_float>(m_iNumWave) * XM_PI);
        const _float fStrength = fRatio * (1.f - fRatio);
        const _float fWave = fShape + fStrength;

        const _vector vOffset = vRight * (m_vWaveHeight * fWave * fAmplitude);

        _float3 vPoint{};
        Math::Store(vPoint, vBase + vOffset);
        m_RopePoints.push_back(vPoint);
    }
}

_bool CRope::Calc_RopeComplete(const _float3& vCurTip)
{
    const _vector vCurDynamicPos = Math::Load(m_vCurDynamicPos);
    const _vector vAnchor = Math::Load(m_vEndPoint);
    const _vector vTip = Math::Load(vCurTip);
    const _vector vTrialDir = Math::Load(m_vTrialDir);

    const _float fDist = Math::Get_X(XMVector3Length(vCurDynamicPos - vAnchor));
    const _vector vCurDir = vTip - vCurDynamicPos;
    const _float fDot = Math::Get_X(XMVector3Dot(vCurDir, vTrialDir));

    if (fDist < 0.1f || fDot < 0.f)
        return true;

    return false;
}

void CRope::Upload_Line()
{
    if (!m_upLine)
        return;

    if (m_RopePoints.size() < 2)
        return;

    m_upLine->Update(m_RopePoints.data(), To<_uint>(m_RopePoints.size()));
    m_upLine->Submit();
}

std::unique_ptr<CRope> CRope::Create(SIDE eSide)
{
    std::unique_ptr<CRope> upRope = std::make_unique<CRope>(eSide);
    upRope->Initialize();
    return upRope;
}

NS_END
