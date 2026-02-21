#include "UIButton.h"

void CUIButton::Set_TargetCanvas(COMPONENT_HANDLE hCanvas)
{
    m_pData->hTargetCanvas = hCanvas;
}

void CUIButton::Set_RectTransform(COMPONENT_HANDLE hRectTransform)
{
    m_pData->hRectTransform = hRectTransform;
}

void CUIButton::Set_Interactable(_bool bInteractable)
{
    m_pData->bInteractable = bInteractable;
    m_pData->eState = bInteractable ? UI_BTN_STATE::Normal : UI_BTN_STATE::Disabled;
}
