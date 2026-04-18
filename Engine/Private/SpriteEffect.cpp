#include "SpriteEffect.h"

CSpriteEffect::CSpriteEffect()
    : CComponent_Proxy_Base()
{
}

CSpriteEffect::CSpriteEffect(DataType* pData, COMPONENT_HANDLE hComponent)
    : CComponent_Proxy_Base(pData, hComponent)

{
}

void CSpriteEffect::Set_Texture(uint32_t hTexture)
{
    if (!m_pData)
        return;
    m_pData->hTexture = hTexture;
}

uint32_t CSpriteEffect::Get_Texture() const
{
    return m_pData ? m_pData->hTexture : INVALID_HANDLE_UINT;
}

void CSpriteEffect::Set_Material(uint32_t hMaterial)
{
    if (!m_pData)
        return;
    m_pData->hMaterial = hMaterial;
}

void CSpriteEffect::Set_FrameInfo(_uint iRow, _uint iCol, _uint iTotalFrame, _float fFrameDuration)
{
    if (!m_pData)
        return;

    m_pData->iRow = iRow;
    m_pData->iCol = iCol;
    m_pData->iTotalFrame = iTotalFrame;
    m_pData->fFrameDuration = fFrameDuration;
}

void CSpriteEffect::Set_Loop(_bool bLoop)
{
    if (!m_pData)
        return;
    m_pData->bLoop = bLoop;
}

void CSpriteEffect::Set_Play(_bool bPlay)
{
    if (!m_pData)
        return;
    m_pData->bPlay = bPlay;
}

void CSpriteEffect::Play()
{
    if (!m_pData)
        return;

    m_pData->bPlay = true;
    m_pData->bFinished = false;
}

void CSpriteEffect::Stop()
{
    if (!m_pData)
        return;

    m_pData->bPlay = false;
}

void CSpriteEffect::Reset()
{
    if (!m_pData)
        return;

    m_pData->fAccTime = 0.f;
    m_pData->iCurFrame = 0;
    m_pData->bFinished = false;
}

void CSpriteEffect::Play_From_Start()
{
    Reset();
    Play();
}

void CSpriteEffect::Set_Billboard(_bool bBillboard)
{
    if (!m_pData)
        return;
    m_pData->bBillboard = bBillboard;
}

void CSpriteEffect::Set_Size(const _float2& vSize)
{
    if (!m_pData)
        return;
    m_pData->vSize = vSize;
}

void CSpriteEffect::Set_Color(const _float4& vColor)
{
    if (!m_pData)
        return;
    m_pData->vColor = vColor;
}

_uint CSpriteEffect::Get_CurrentFrame() const
{
    return m_pData ? m_pData->iCurFrame : 0;
}

_bool CSpriteEffect::Is_Finished() const
{
    return m_pData ? m_pData->bFinished : true;
}
