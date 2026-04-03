#pragma once
#include "Client_Define.h"
#include "Titan_Struct.h"

NS_BEGIN(Client)

class CTitanBound_Controller final : public IScript
{
public:
    CTitanBound_Controller();
    ~CTitanBound_Controller() override;

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;
    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private:
    struct TITAN_BOUND_NODE
    {
        SCRIPT_OBJECT_REF   refObject{};
        Engine::CGameObject* pObject{};
        CTransform          trObject{};
        _float3* pOffset{};
    };

private:
    void Register_Bound(std::vector<TITAN_BOUND_NODE>& vecBounds, SCRIPT_OBJECT_REF& refBound, _float3& vOffset);
    void Sync_Bound(TITAN_BOUND_NODE& tBound, _fvector vOwnerPos, _fvector vOwnerRot) const;

private:
    void Bind_Trigger(const SCRIPT_OBJECT_REF& refBound, void (CTitanBound_Controller::* pFunc)(const COLLISION_DESC&));
    void Try_QueueGrabAnim(const char* pAnimName, const COLLISION_DESC& tDesc);

    void OnTriggerEnter_Weak(const COLLISION_DESC& tDesc);

    void OnTriggerEnter_GrabAirFarL(const COLLISION_DESC& tDesc);
    void OnTriggerEnter_GrabAirFarR(const COLLISION_DESC& tDesc);

    void OnTriggerEnter_GrabAirShortL(const COLLISION_DESC& tDesc);
    void OnTriggerEnter_GrabAirShortR(const COLLISION_DESC& tDesc);

    void OnTriggerEnter_GrabBackL(const COLLISION_DESC& tDesc);
    void OnTriggerEnter_GrabBackR(const COLLISION_DESC& tDesc);

    void OnTriggerEnter_GrabGroundBackL(const COLLISION_DESC& tDesc);
    void OnTriggerEnter_GrabGroundBackR(const COLLISION_DESC& tDesc);

    void OnTriggerEnter_GrabGroundFrontL(const COLLISION_DESC& tDesc);
    void OnTriggerEnter_GrabGroundFrontR(const COLLISION_DESC& tDesc);

    void OnTriggerEnter_GrabHeadBackL(const COLLISION_DESC& tDesc);
    void OnTriggerEnter_GrabHeadBackR(const COLLISION_DESC& tDesc);

    void OnTriggerEnter_GrabHeadFrontL(const COLLISION_DESC& tDesc);
    void OnTriggerEnter_GrabHeadFrontR(const COLLISION_DESC& tDesc);

    void OnTriggerEnter_GrabHighL(const COLLISION_DESC& tDesc);
    void OnTriggerEnter_GrabHighR(const COLLISION_DESC& tDesc);

    void OnTriggerEnter_GrabStomachL(const COLLISION_DESC& tDesc);
    void OnTriggerEnter_GrabStomachR(const COLLISION_DESC& tDesc);

public:
    _bool Consume_PendingGrabAnim(std::string& strOutAnim);
    void Clear_PendingGrabAnim();
    void Set_GrabTriggerEnabled(_bool bEnable);

private:
    std::string m_strPendingGrabAnim{};
    _bool       m_bPendingGrabAnim = false;
    _bool       m_bGrabTriggerEnabled = true;

private:
    Engine::CGameObject* m_pOwner{};
    CTransform                      m_trOwner{};
    std::vector<TITAN_BOUND_NODE>   m_vecBounds;
public:
    SCRIPT_FIELDS_BEGIN(CTitanBound_Controller)
        SCRIPT_FIELD_OBJECT_REF(m_refOwner)

        SCRIPT_FIELD_OBJECT_REF(m_refWeak)
        SCRIPT_FIELD_FLOAT3(m_vWeakOffset)

        SCRIPT_FIELD_OBJECT_REF(m_refGrabAirFarL)
        SCRIPT_FIELD_FLOAT3(m_vGrabAirFarLOffset)
        SCRIPT_FIELD_OBJECT_REF(m_refGrabAirFarR)
        SCRIPT_FIELD_FLOAT3(m_vGrabAirFarROffset)

        SCRIPT_FIELD_OBJECT_REF(m_refGrabAirShortL)
        SCRIPT_FIELD_FLOAT3(m_vGrabAirShortLOffset)
        SCRIPT_FIELD_OBJECT_REF(m_refGrabAirShortR)
        SCRIPT_FIELD_FLOAT3(m_vGrabAirShortROffset)

        SCRIPT_FIELD_OBJECT_REF(m_refGrabBackL)
        SCRIPT_FIELD_FLOAT3(m_vGrabBackLOffset)
        SCRIPT_FIELD_OBJECT_REF(m_refGrabBackR)
        SCRIPT_FIELD_FLOAT3(m_vGrabBackROffset)

        SCRIPT_FIELD_OBJECT_REF(m_refGrabGroundBackL)
        SCRIPT_FIELD_FLOAT3(m_vGrabGroundBackLOffset)
        SCRIPT_FIELD_OBJECT_REF(m_refGrabGroundBackR)
        SCRIPT_FIELD_FLOAT3(m_vGrabGroundBackROffset)

        SCRIPT_FIELD_OBJECT_REF(m_refGrabGroundFrontL)
        SCRIPT_FIELD_FLOAT3(m_vGrabGroundFrontLOffset)
        SCRIPT_FIELD_OBJECT_REF(m_refGrabGroundFrontR)
        SCRIPT_FIELD_FLOAT3(m_vGrabGroundFrontROffset)

        SCRIPT_FIELD_OBJECT_REF(m_refGrabHeadBackL)
        SCRIPT_FIELD_FLOAT3(m_vGrabHeadBackLOffset)
        SCRIPT_FIELD_OBJECT_REF(m_refGrabHeadBackR)
        SCRIPT_FIELD_FLOAT3(m_vGrabHeadBackROffset)

        SCRIPT_FIELD_OBJECT_REF(m_refGrabHeadFrontL)
        SCRIPT_FIELD_FLOAT3(m_vGrabHeadFrontLOffset)
        SCRIPT_FIELD_OBJECT_REF(m_refGrabHeadFrontR)
        SCRIPT_FIELD_FLOAT3(m_vGrabHeadFrontROffset)

        SCRIPT_FIELD_OBJECT_REF(m_refGrabHighL)
        SCRIPT_FIELD_FLOAT3(m_vGrabHighLOffset)
        SCRIPT_FIELD_OBJECT_REF(m_refGrabHighR)
        SCRIPT_FIELD_FLOAT3(m_vGrabHighROffset)

        SCRIPT_FIELD_OBJECT_REF(m_refGrabStomachL)
        SCRIPT_FIELD_FLOAT3(m_vGrabStomachLOffset)
        SCRIPT_FIELD_OBJECT_REF(m_refGrabStomachR)
        SCRIPT_FIELD_FLOAT3(m_vGrabStomachROffset)
        SCRIPT_FIELDS_END(CTitanBound_Controller)

private:
    SCRIPT_OBJECT_REF               m_refOwner{};
    SCRIPT_OBJECT_REF               m_refWeak{};
    _float3                         m_vWeakOffset{ 0.f, 0.f, 0.f };

    SCRIPT_OBJECT_REF               m_refGrabAirFarL{};
    _float3                         m_vGrabAirFarLOffset{ 0.f, 0.f, 0.f };
    SCRIPT_OBJECT_REF               m_refGrabAirFarR{};
    _float3                         m_vGrabAirFarROffset{ 0.f, 0.f, 0.f };

    SCRIPT_OBJECT_REF               m_refGrabAirShortL{};
    _float3                         m_vGrabAirShortLOffset{ 0.f, 0.f, 0.f };
    SCRIPT_OBJECT_REF               m_refGrabAirShortR{};
    _float3                         m_vGrabAirShortROffset{ 0.f, 0.f, 0.f };

    SCRIPT_OBJECT_REF               m_refGrabBackL{};
    _float3                         m_vGrabBackLOffset{ 0.f, 0.f, 0.f };
    SCRIPT_OBJECT_REF               m_refGrabBackR{};
    _float3                         m_vGrabBackROffset{ 0.f, 0.f, 0.f };

    SCRIPT_OBJECT_REF               m_refGrabGroundBackL{};
    _float3                         m_vGrabGroundBackLOffset{ 0.f, 0.f, 0.f };
    SCRIPT_OBJECT_REF               m_refGrabGroundBackR{};
    _float3                         m_vGrabGroundBackROffset{ 0.f, 0.f, 0.f };

    SCRIPT_OBJECT_REF               m_refGrabGroundFrontL{};
    _float3                         m_vGrabGroundFrontLOffset{ 0.f, 0.f, 0.f };
    SCRIPT_OBJECT_REF               m_refGrabGroundFrontR{};
    _float3                         m_vGrabGroundFrontROffset{ 0.f, 0.f, 0.f };

    SCRIPT_OBJECT_REF               m_refGrabHeadBackL{};
    _float3                         m_vGrabHeadBackLOffset{ 0.f, 0.f, 0.f };
    SCRIPT_OBJECT_REF               m_refGrabHeadBackR{};
    _float3                         m_vGrabHeadBackROffset{ 0.f, 0.f, 0.f };

    SCRIPT_OBJECT_REF               m_refGrabHeadFrontL{};
    _float3                         m_vGrabHeadFrontLOffset{ 0.f, 0.f, 0.f };
    SCRIPT_OBJECT_REF               m_refGrabHeadFrontR{};
    _float3                         m_vGrabHeadFrontROffset{ 0.f, 0.f, 0.f };

    SCRIPT_OBJECT_REF               m_refGrabHighL{};
    _float3                         m_vGrabHighLOffset{ 0.f, 0.f, 0.f };
    SCRIPT_OBJECT_REF               m_refGrabHighR{};
    _float3                         m_vGrabHighROffset{ 0.f, 0.f, 0.f };

    SCRIPT_OBJECT_REF               m_refGrabStomachL{};
    _float3                         m_vGrabStomachLOffset{ 0.f, 0.f, 0.f };
    SCRIPT_OBJECT_REF               m_refGrabStomachR{};
    _float3                         m_vGrabStomachROffset{ 0.f, 0.f, 0.f };
};

NS_END
