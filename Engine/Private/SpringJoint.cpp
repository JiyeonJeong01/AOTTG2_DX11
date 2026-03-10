#include "SpringJoint.h"
#include "Engine_Math.h"

void CSpringJoint::Set_UseSpring(_bool bUseSpring)
{
    if (!m_pData)
        return;

    m_pData->bUseSpring = bUseSpring;
}

_bool CSpringJoint::Get_UseSpring() const
{
    if (!m_pData)
        return false;

    return m_pData->bUseSpring;
}

void CSpringJoint::Set_Anchor(const _float3& vAnchor)
{
    if (!m_pData)
        return;

    m_pData->vAnchor = vAnchor;
}

_float3 CSpringJoint::Get_Anchor() const
{
    if (!m_pData)
        return Math::Zero3();

    return m_pData->vAnchor;
}

void CSpringJoint::Set_Spring(_float fSpring)
{
    if (!m_pData)
        return;

    if (fSpring < 0.f)
        fSpring = 0.f;

    m_pData->fSpring = fSpring;
}

_float CSpringJoint::Get_Spring() const
{
    if (!m_pData)
        return 0.f;

    return m_pData->fSpring;
}

void CSpringJoint::Set_Damper(_float fDamper)
{
    if (!m_pData)
        return;

    if (fDamper < 0.f)
        fDamper = 0.f;

    m_pData->fDamper = fDamper;
}

_float CSpringJoint::Get_Damper() const
{
    if (!m_pData)
        return 0.f;

    return m_pData->fDamper;
}

void CSpringJoint::Set_RestLength(_float fRestLength)
{
    if (!m_pData)
        return;

    if (fRestLength < 0.f)
        fRestLength = 0.f;

    m_pData->fRestLength = fRestLength;
}

_float CSpringJoint::Get_RestLength() const
{
    if (!m_pData)
        return 0.f;

    return m_pData->fRestLength;
}

void CSpringJoint::Set_MinLength(_float fMinLength)
{
    if (!m_pData)
        return;

    if (fMinLength < 0.f)
        fMinLength = 0.f;

    m_pData->fMinLength = fMinLength;
}

_float CSpringJoint::Get_MinLength() const
{
    if (!m_pData)
        return 0.f;

    return m_pData->fMinLength;
}

void CSpringJoint::Set_MaxLength(_float fMaxLength)
{
    if (!m_pData)
        return;

    if (fMaxLength < 0.f)
        fMaxLength = 0.f;

    m_pData->fMaxLength = fMaxLength;
}

_float CSpringJoint::Get_MaxLength() const
{
    if (!m_pData)
        return 0.f;

    return m_pData->fMaxLength;
}

void CSpringJoint::Set_UseMinLength(_bool bUseMinLength)
{
    if (!m_pData)
        return;

    m_pData->bUseMinLength = bUseMinLength;
}

_bool CSpringJoint::Get_UseMinLength() const
{
    if (!m_pData)
        return false;

    return m_pData->bUseMinLength;
}

void CSpringJoint::Set_UseMaxLength(_bool bUseMaxLength)
{
    if (!m_pData)
        return;

    m_pData->bUseMaxLength = bUseMaxLength;
}

_bool CSpringJoint::Get_UseMaxLength() const
{
    if (!m_pData)
        return false;

    return m_pData->bUseMaxLength;
}
