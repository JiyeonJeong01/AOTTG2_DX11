#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)
class CUI_BladeController;
class CUI_GasController;
class CUI_SkillController;
class CODM_Gear;
NS_END

NS_BEGIN(Client)

class CHUDController : public IScript
{

private :
    CUI_BladeController*    m_pBladeCtrl = nullptr;
    CUI_GasController*      m_pGasCtrl = nullptr;
    CUI_SkillController*    m_pSkill = nullptr;

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    CGameObject*            m_goCursor{};
    CUIText                 m_txtCursor{};

    CODM_Gear*              m_pGear{};
    const _float4           m_vValidTargetColor = { 1.f, 1.f, 1.f, 1.f };
    const _float4           m_vInvalidTargetColor = { 1.f, 0.f, 0.f, 1.f };

    _bool                   m_bInitialized = false;

public :
    SCRIPT_OBJECT_REF       m_refPlayer;
    SCRIPT_OBJECT_REF       m_refEren;
    SCRIPT_OBJECT_REF       m_refCursor;

SCRIPT_FIELDS_BEGIN(CHUDController)
    SCRIPT_FIELD_OBJECT_REF(m_refPlayer)
    SCRIPT_FIELD_OBJECT_REF(m_refEren)
    SCRIPT_FIELD_OBJECT_REF(m_refCursor)
SCRIPT_FIELDS_END(CHUDController)
};

NS_END;
