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

public:
    void On_PlayerHitTitan();
    void On_PlayerKillTitan(_float fAccuracyScore);

private:
    void Set_References();

    void Set_BloodVisible(_bool bVisible);
    void Set_BloodAlpha(_uint iIndex, _float fAlpha);
    void Update_BloodEffect(_float fDT);

    void Set_KillTextVisible(_bool bVisible);
    void Set_KillTextAlpha(_float fAlpha);
    void Set_KillTextString(const std::wstring& strText);
    void Reset_KillTextPosition();
    void Update_KillTextEffect(_float fDT);

    std::wstring Build_AccuracyText(_float fAccuracyScore) const;

private:
    SCRIPT_OBJECT_REF   m_refBloodImage0{};
    SCRIPT_OBJECT_REF   m_refBloodImage1{};
    SCRIPT_OBJECT_REF   m_refBloodImage2{};

    SCRIPT_OBJECT_REF   m_refDamageText0{};
    SCRIPT_OBJECT_REF   m_refDamageText1{};

private:
    CGameObject*        m_goBloodImage0 = nullptr;
    CGameObject*        m_goBloodImage1 = nullptr;
    CGameObject*        m_goBloodImage2 = nullptr;

    CCanvasRenderer     m_crBloodImage0{};
    CCanvasRenderer     m_crBloodImage1{};
    CCanvasRenderer     m_crBloodImage2{};

    CGameObject*        m_goDamageText0 = nullptr;
    CGameObject*        m_goDamageText1 = nullptr;

    CUIText             m_txtDamageText0{};
    CUIText             m_txtDamageText1{};

    CRectTransform      m_rtDamageText0{};
    CRectTransform      m_rtDamageText1{};

private:
    _bool               m_bBloodPlaying = false;
    _float              m_fBloodElapsed = 0.f;

    _float              m_fBloodSequenceInterval = 0.045f;  /* 0 -> 1 -> 2 순서대로 배치 */
    const _float        m_fBloodHoldTime = 0.22f;           /* blood 이미지 유지 시간 */
    const _float        m_fBloodFadeDuration = 0.35f;       /* 사라지는 시간 */

private:
    _bool               m_bKillTextPlaying = false;         
    _float              m_fKillTextElapsed = 0.f;

    const _float        m_fKillTextRiseInDuration = 0.5f;  /* 등장 시간 */
    const _float        m_fKillTextHoldDuration = 1.2f;    /* text 유지 시간 */
    const _float        m_fKillTextFadeOutDuration = 0.25f; /* 사라지는 시간 */

    const _float2       m_vDamageText0StartLocalPos = { 1200.f, 600.f };
    const _float2       m_vDamageText1StartLocalPos = { 1205.f, 605.f };

    const _float2       m_vDamageTextRiseOffset = { 0.f, -50.f };    /* 등장 시작 local ~ 유지 위치 */
    const _float2       m_vDamageTextFadeOffset = { 0.f, -25.f };    /* 유지 ~ fade 위치 */

    _float4             m_vDamageTextColor0 = { 0.f, 0.f, 0.f, 1.f };
    _float4             m_vDamageTextColor1 = { 0.f, 0.f, 0.f, 1.f };

    SCRIPT_FIELDS_BEGIN(CUI_HitController)
        SCRIPT_FIELD_OBJECT_REF(m_refBloodImage0)
        SCRIPT_FIELD_OBJECT_REF(m_refBloodImage1)
        SCRIPT_FIELD_OBJECT_REF(m_refBloodImage2)

        SCRIPT_FIELD_OBJECT_REF(m_refDamageText0)
        SCRIPT_FIELD_OBJECT_REF(m_refDamageText1)
    SCRIPT_FIELDS_END(CUI_HitController)
};

NS_END;
