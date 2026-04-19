#pragma once
#include "Client_Define.h"

NS_BEGIN(Client)

class CResupplyStation final : public IScript
{

public:
    CResupplyStation();
    ~CResupplyStation() override;

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

public:
    _bool Is_PlayerDetected() const
    {
        return m_bPlayerDetected;
    }

    CGameObject* Get_DetectedPlayer() const
    {
        return m_goDetectedPlayer;
    }

    _float3 Get_UIWorldPosition() const;

public:
    void OnTriggerEnter(const COLLISION_DESC& tCollisionDesc);
    void OnTriggerExit(const COLLISION_DESC& tCollisionDesc);

private:
    HRESULT SetUp_References();
    _bool   Is_Player(const CGameObject* pOther) const;
    void    On_DetectedPlayer(_bool bDetected, CGameObject* pPlayer);
    void    Update_UIPosition();
    void    Set_UIVisible(_bool bVisible);
    _bool   Project_WorldToScreen(const _float3& vWorldPos, _float2& vOutScreenPos) const;
    void    Update_UISelection();
    void    Apply_UISelectionVisual();
    void    Try_Interact();

private:
    CGameObject*    m_goOwner = nullptr;
    CGameObject*    m_goUILeftObject = nullptr;
    CGameObject*    m_goUIRightObject = nullptr;
    CGameObject*    m_goUILeftTextObject = nullptr;
    CGameObject*    m_goUIRightTextObject = nullptr;
    CGameObject*    m_goDetectedPlayer = nullptr;

    CTransform      m_trUIPivot;
    CRectTransform  m_rtUILeft;
    CRectTransform  m_rtUIRight;
    CRectTransform  m_rtUITextLeft;
    CRectTransform  m_rtUITextRight;

    _bool           m_bPlayerDetected = false;
    _bool           m_bUpdatedPostition = false;

    SIDE            m_eSide = SIDE::LEFT;

    _float          m_fSelectedUIScale = 1.15f;
    _float          m_fUnselectedUIScale = 1.f;

    _float          m_fSelectedTextScale = 1.15f;
    _float          m_fUnselectedTextScale = 1.f;

    const _float2   m_vBaseSize = { 200.f, 100.f };
    _float2         m_vUILeftBaseSize = { 0.f, 0.f };
    _float2         m_vUIRightBaseSize = { 0.f, 0.f };


private:
    SCRIPT_OBJECT_REF   m_refUILeftObject{};
    SCRIPT_OBJECT_REF   m_refUIRightObject{};
    SCRIPT_OBJECT_REF   m_refUITextLeftObject{};
    SCRIPT_OBJECT_REF   m_refUITextRightObject{};

    _float              m_fUIHeightOffset = 65.f;
    _float              m_fUIHalfSpacing = 40.f;
    _float              m_fOutlineWidth = 5.f;

public:
    SCRIPT_FIELDS_BEGIN(CResupplyStation)
        SCRIPT_FIELD_OBJECT_REF(m_refUILeftObject)
        SCRIPT_FIELD_OBJECT_REF(m_refUIRightObject)
        SCRIPT_FIELD_OBJECT_REF(m_refUITextLeftObject)
        SCRIPT_FIELD_OBJECT_REF(m_refUITextRightObject)
        SCRIPT_FIELD_FLOAT(m_fUIHeightOffset)
        SCRIPT_FIELD_FLOAT(m_fUIHalfSpacing)
        SCRIPT_FIELDS_END(CResupplyStation)

};

NS_END
