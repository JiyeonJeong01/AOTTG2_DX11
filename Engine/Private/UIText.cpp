#include "UIText.h"

void CUIText::Set_CanvasRenderer(COMPONENT_HANDLE hCanvas)
{
    if (!m_pData)
        return;

    m_pData->hCanvasRenderer = hCanvas;
    m_pData->dirty = true;
}

void CUIText::Set_Font(uint32_t hFont)
{
    if (!m_pData)
        return;

    m_pData->hFont = hFont;
    m_pData->dirty = true;
}

void CUIText::Set_Text(const std::basic_string<_tchar>& strText)
{
    if (!m_pData)
        return;

    m_pData->strText = strText;
    m_pData->dirty = true;
}

void CUIText::Set_Text(const _tchar* pText)
{
    if (!m_pData)
        return;

    m_pData->strText = (pText != nullptr) ? pText : TEXT("");
    m_pData->dirty = true;
}

void CUIText::Set_Color(const _float4& vColor)
{
    if (!m_pData)
        return;

    m_pData->color = vColor;
    m_pData->dirty = true;
}

void CUIText::Set_Scale(_float fScale)
{
    if (!m_pData)
        return;

    m_pData->fScale = fScale;
    m_pData->dirty = true;
}

void CUIText::Set_VisualPriority(uint8_t p)
{
    if (!m_pData)
        return;

    m_pData->visualPriority = p;
    m_pData->dirty = true;
}

const std::basic_string<_tchar>& CUIText::Get_Text() const
{
    static const std::basic_string<_tchar> strEmpty{};

    if (!m_pData)
        return strEmpty;

    return m_pData->strText;
}
