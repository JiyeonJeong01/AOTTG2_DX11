#include "Camera.h"

void CCamera::Set_Priority(uint8_t iPriority)
{
    if (nullptr == m_pData)
        return;

    m_pData->iPriority = iPriority; 
    m_pData->dirty = true;
}

void CCamera::Set_Ortho(_bool bOrthographic)
{
    if (nullptr == m_pData)
        return;

    m_pData->bOrthographic = bOrthographic;
    m_pData->dirty = true;
}

void CCamera::Set_Fovy(_float fFovy)
{
    if (nullptr == m_pData)
        return;

    m_pData->fFovy = fFovy;
    m_pData->dirty = true;
}

void CCamera::Set_OrthoSize(_float fOrthoSize)
{
    if (nullptr == m_pData)
        return;

    m_pData->fOrthoSize = fOrthoSize;
    m_pData->dirty = true;
}

void CCamera::Set_Aspect(_float fAspect)
{
    if (nullptr == m_pData)
        return;

    m_pData->fAspect = fAspect;
    m_pData->dirty = true;
}

void CCamera::Set_NearFar(_float fNear, _float fFar)
{
    if (nullptr == m_pData)
        return;

    m_pData->fNear = fNear;
    m_pData->fFar = fFar;
    m_pData->dirty = true;
}

void CCamera::Set_LayerMask(uint32_t uintLayerMask)
{
    if (nullptr == m_pData)
        return;

    m_pData->layerMask = uintLayerMask;
    m_pData->dirty = true;
}
