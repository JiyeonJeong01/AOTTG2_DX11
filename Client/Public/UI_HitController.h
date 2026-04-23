#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)

class CUI_HitController : public IScript
{
public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    //void Set_References();
    ////      CGameObject* goHUD = GAME_INSTANCE.Find_GameObject(m_refHUDController.hObject); 이렇게 찾아오기
    ////      m_crDialoguePanel = m_goDialogueUI->Get_Component<CCanvasRenderer>();
    ////      m_txtDialogue = m_goDialogueUI->Get_Component<CUIText>(); 
    ////      m_Skill_Img = pObj->Get_Component<CUIImage>();

    //void On_HitTitan();
    //void On_KilledTitan();


private :
    SCRIPT_OBJECT_REF   m_refBlookImage0{};
    SCRIPT_OBJECT_REF   m_refBlookImage1{};
    SCRIPT_OBJECT_REF   m_refBlookImage2{};

    SCRIPT_OBJECT_REF   m_refDamageText{};

    SCRIPT_FIELDS_BEGIN(CUI_HitController)
        SCRIPT_FIELD_OBJECT_REF(m_refBlookImage0)
        SCRIPT_FIELD_OBJECT_REF(m_refBlookImage1)
        SCRIPT_FIELD_OBJECT_REF(m_refBlookImage2)
        SCRIPT_FIELD_OBJECT_REF(m_refDamageText)
    SCRIPT_FIELDS_END(CUI_HitController)
};

NS_END;
