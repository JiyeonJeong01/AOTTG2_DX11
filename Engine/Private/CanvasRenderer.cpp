#include "CanvasRenderer.h"

NS_BEGIN(Engine)

CCanvasRenderer::CCanvasRenderer()
    : CComponent_Proxy_Base()
{
}

CCanvasRenderer::CCanvasRenderer(DataType* pData, COMPONENT_HANDLE handle)
    : CComponent_Proxy_Base(pData, handle)
{
}

void CCanvasRenderer::Set_Material(uint32_t h)
{
    m_pData->hMaterial = h;
}
void CCanvasRenderer::Set_Texture(uint32_t h)
{
    m_pData->hTexture = h;
}

void CCanvasRenderer::Set_Color(const _float4& vColor)
{
    m_pData->vColor = vColor;
}
void CCanvasRenderer::Set_UV(const RECT_F& rcUV)
{
    m_pData->rcUV = rcUV;
}

void CCanvasRenderer::Enable_ClipRect(_bool b)
{
    if (b) m_pData->flags |= CF_CLIP_RECT;
    else   m_pData->flags &= ~CF_CLIP_RECT;
}

void CCanvasRenderer::Set_ClipRect(const RECT_F& rcClip)
{
    m_pData->rcClip = rcClip;
}

void CCanvasRenderer::Set_Layer(RENDER_LAYER e)
{
    m_pData->layer = e;
}
void CCanvasRenderer::Set_Flags(uint32_t f)
{
    m_pData->flags = f;
}
void CCanvasRenderer::Add_Flags(uint32_t f)
{
    m_pData->flags |= f;
}
void CCanvasRenderer::Remove_Flags(uint32_t f)
{
    m_pData->flags &= ~f;
}
void CCanvasRenderer::Set_SortZ(float z)
{
    m_pData->sortZ = z;
}

COMPONENT_HANDLE CCanvasRenderer::Get_RectTransform() const
{
    return m_pData->hRectTransform;
}
uint32_t CCanvasRenderer::Get_Material() const { return
    m_pData->hMaterial; }
uint32_t CCanvasRenderer::Get_Texture() const
{
    return m_pData->hTexture;
}
uint32_t CCanvasRenderer::Get_Flags() const
{
    return m_pData->flags;
}
RENDER_LAYER CCanvasRenderer::Get_Layer() const
{
    return m_pData->layer;
}
float CCanvasRenderer::Get_SortZ() const
{
    return m_pData->sortZ;
}

const _float4& CCanvasRenderer::Get_Color() const
{
    return m_pData->vColor;
}
const RECT_F& CCanvasRenderer::Get_UV() const
{
    return m_pData->rcUV;
}
const RECT_F& CCanvasRenderer::Get_ClipRect() const
{
    return m_pData->rcClip;
}

NS_END
