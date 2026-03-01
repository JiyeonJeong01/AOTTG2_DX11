#include "Light.h"
void CLight::Set_Enabled(_bool bEnable)
{
    if (nullptr == m_pData)
        return;

    m_pData->bEnabled = bEnable ? 1 : 0;
    m_pData->dirty = true;
}

void CLight::Set_Type(LIGHT_TYPE eType)
{
    if (nullptr == m_pData)
        return;

    m_pData->type = eType;
    m_pData->dirty = true;
}

void CLight::Set_Color(const _float4& vColor)
{
    if (nullptr == m_pData)
        return;

    m_pData->vColor = vColor;
    m_pData->dirty = true;
}

void CLight::Set_Range(_float fRange)
{
    if (nullptr == m_pData)
        return;

    m_pData->fRange = fRange;
    m_pData->dirty = true;
}

void CLight::Set_SpotAngle(_float fSpotAngle)
{
    if (nullptr == m_pData)
        return;

    m_pData->spotAngle = fSpotAngle;
    m_pData->dirty = true;
}

void CLight::Set_Diffuse(const _float4& vDiffuse)
{
    if (nullptr == m_pData)
        return;

    m_pData->vDiffuse = vDiffuse;
    m_pData->dirty = true;
}

void CLight::Set_Ambient(const _float4& vAmbient)
{
    if (nullptr == m_pData)
        return;

    m_pData->vAmbient = vAmbient;
    m_pData->dirty = true;
}

void CLight::Set_Specular(const _float4& vSpecular)
{
    if (nullptr == m_pData)
        return;

    m_pData->vSpecular = vSpecular;
    m_pData->dirty = true;
}
