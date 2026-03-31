#pragma once
#include "Client_Define.h"
#include "Player_Struct.h"
#include "Script.h"

NS_BEGIN(Client)

class CPlayer_SkillController;

class CUI_SkillController : public IScript
{
public :
    SCRIPT_OBJECT_REF   m_refPlayer{};

    SCRIPT_OBJECT_REF   m_tRefSkill_Image{};
    SCRIPT_OBJECT_REF   m_tRefSpinH_CoolDown_Image{};
    SCRIPT_OBJECT_REF   m_tRefThrow_CoolDown_Image{};
    SCRIPT_OBJECT_REF   m_tRefSpinV_CoolDown_Image{};

    ASSET_GUID          m_SpinH_Texture{};
    ASSET_GUID          m_Throw_Texture{};
    ASSET_GUID          m_SpinV_Texture{};
    ASSET_GUID          m_CoolDownSmall{};
    ASSET_GUID          m_CoolDownBig{};

    _float2              m_vCoolDownSmallPos[To<_uint>(SKILL_TYPE::END)] = {
        {1556.f, 972.f}, {1671.f, 972.f}, {1766.f, 972.f}
    };
    _float2              m_vCoolDownBigPos[To<_uint>(SKILL_TYPE::END)] = {
        {1566.f, 966.f}, {1664.f, 966.f}, {1760.f, 966.f}
    };
    _float2             m_vCoolDownSmallSize{ 79.f, 65.f };
    _float2             m_vCoolDownBigSize{ 91.f, 71.f };

SCRIPT_FIELDS_BEGIN(CUI_SkillController)
    SCRIPT_FIELD_OBJECT_REF(m_refPlayer)

    SCRIPT_FIELD_OBJECT_REF(m_tRefSkill_Image)

    SCRIPT_FIELD_FLOAT2(m_vCoolDownSmallSize)
    SCRIPT_FIELD_FLOAT2(m_vCoolDownBigSize)

    SCRIPT_FIELD_OBJECT_REF(m_tRefSpinH_CoolDown_Image)
    SCRIPT_FIELD_OBJECT_REF(m_tRefThrow_CoolDown_Image)
    SCRIPT_FIELD_OBJECT_REF(m_tRefSpinV_CoolDown_Image)

    SCRIPT_FIELD_ASSET_GUID(m_SpinH_Texture)
    SCRIPT_FIELD_ASSET_GUID(m_Throw_Texture)
    SCRIPT_FIELD_ASSET_GUID(m_SpinV_Texture)

    SCRIPT_FIELD_ASSET_GUID(m_CoolDownSmall)
    SCRIPT_FIELD_ASSET_GUID(m_CoolDownBig)
SCRIPT_FIELDS_END(CUI_SkillController)

private :
    CPlayer_SkillController*    m_pSkillController = nullptr;
    PLAYER_SKILL*               m_pCurSkill{};

    CUIImage                    m_Skill_Img;

private:
    CRectTransform               m_SpinH_CoolDown_RT;
    CRectTransform               m_Throw_CoolDown_RT;
    CRectTransform               m_SpinV_CoolDown_RT;

    CCanvasRenderer                    m_crSpinHCoolDown;
    CCanvasRenderer                    m_crThrowCoolDown;
    CCanvasRenderer                    m_crSpinVCoolDown;

    uint32_t                    m_hSpinH = INVALID_HANDLE_UINT;
    uint32_t                    m_hThrow = INVALID_HANDLE_UINT;
    uint32_t                    m_hSpinV = INVALID_HANDLE_UINT;
    uint32_t                    m_hCoolDownSmall = INVALID_HANDLE_UINT;
    uint32_t                    m_hCoolDownBig = INVALID_HANDLE_UINT;

private :
    void Apply_CoolDownState(CCanvasRenderer& tImg, CRectTransform& tRT, SKILL_TYPE eTargetSkillType, SKILL_TYPE eActiveSkillType);
    void Refresh_AllCoolDownUI();

    void On_ActiveSkillChanged(SKILL_TYPE eSkillType);

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;
};



NS_END;
