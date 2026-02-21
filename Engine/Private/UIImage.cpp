#include "UIImage.h"

void CUIImage::Set_CanvasRenderer(COMPONENT_HANDLE hCanvas)
{
    m_pData->hCanvasRenderer = hCanvas;
    m_pData->dirty = true;
}

void CUIImage::Set_Texture(uint32_t hTex)
{
    m_pData->hTexture = hTex;
    m_pData->dirty = true;
}

void CUIImage::Set_UV(const RECT_F& rcUV)
{
    m_pData->rcUV = rcUV;
    m_pData->dirty = true;
}

void CUIImage::Set_Color(const _float4& vColor)
{
    m_pData->color = vColor;
    m_pData->dirty = true;
}

void CUIImage::Set_VisualPriority(uint8_t p)
{
    m_pData->visualPriority = p;
    m_pData->dirty = true;
}
