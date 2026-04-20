#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)

enum class NOTICE_TYPE : uint32_t
{
    NONE = 0,
    SAVE_EREN,
    RESUPPLY_GAS,
    END
};

class CUI_NoticeController : public IScript
{
public:
    CUI_NoticeController();
    ~CUI_NoticeController() override = default;

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

public:
    void Show_Notice(NOTICE_TYPE eType, _float fDelayTime);

private:
    enum class NOTICE_STATE : uint32_t
    {
        IDLE = 0,
        ENTER,
        HOLD,
        EXIT
    };

private:
    void Initialize_NoticeText();
    void Set_NoticeText(NOTICE_TYPE eType);
    void Set_UIActive(_bool bEnable);
    void Set_UIPosition(const _float2& vPos);
    void Update_NoticeState(_float fDT);

private:
    SCRIPT_OBJECT_REF   m_refNoticeRoot;

    _float2             m_vHiddenPos = { 0.f, -120.f};
    _float2             m_vShownPos = { 0.f, 80.f };

    _float              m_fEnterDuration = 1.f;
    _float              m_fExitDuration = 1.f;

private:
    CGameObject*        m_pNoticeRootObject = nullptr;

    CRectTransform      m_rtNotice;
    CUIText             m_txtNotice;

private:
    NOTICE_STATE        m_eState = NOTICE_STATE::IDLE;
    NOTICE_TYPE         m_eCurNoticeType = NOTICE_TYPE::NONE;

    _float              m_fStateTime = 0.f;
    _float              m_fHoldTime = 0.f;
    _bool               m_bPlaying = false;

private:
    std::vector<std::wstring> m_strNotice;

public:
    SCRIPT_FIELDS_BEGIN(CUI_NoticeController)
        SCRIPT_FIELD_OBJECT_REF(m_refNoticeRoot)
        SCRIPT_FIELD_FLOAT2(m_vHiddenPos)
        SCRIPT_FIELD_FLOAT2(m_vShownPos)
        SCRIPT_FIELD_FLOAT(m_fEnterDuration)
        SCRIPT_FIELD_FLOAT(m_fExitDuration)
        SCRIPT_FIELDS_END(CUI_NoticeController)
};

NS_END
