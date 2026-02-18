#include "RectTransform.h"

void CRectTransform::Set_PositionPx(_float x, _float y)
{
    m_pData->vPosPx = { x, y };
    m_pData->bDirty = true;
}

void CRectTransform::Set_SizePx(_float w, _float h)
{
    m_pData->vSizePx = { w, h };
    m_pData->bDirty = true;
}

// --- Getters ---
const _float4x4& CRectTransform::Get_World()
{
    return m_pData->matWorld;
}

_float2 CRectTransform::Get_PositionPx() const
{
    return m_pData->vPosPx;
}
_float2 CRectTransform::Get_SizePx() const
{
    return m_pData->vSizePx;
}
