#pragma once
#include "Client_Define.h"
#include "Rope.h"
#include "Event.h"
#include "Player_Struct.h"

#pragma region FD

NS_BEGIN(Client)
class CRope;
typedef struct tagTryGrapplingInfo
{
    _float3 vPoint{};
    _float3 vCamOrigin{};
    _float3 vRayDir{};
    _float  fDist{};

    _bool           bDynamicAnchor = false;
    OBJECT_HANDLE   hAnchorObject{};
    _float3         vLocalAnchorOffset{};
} TRY_GRAPPLING_INFO;

typedef struct tagGasState
{
    const _float    fMax = 100.f;
    _float          fCurrent = fMax;
} GAS_STATE;

NS_END
#pragma endregion

NS_BEGIN(Client)

class CODM_Gear : public IScript
{
public :
    CODM_Gear();
    ~CODM_Gear();

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    std::unique_ptr<CRope>  m_upLeftRope;
    std::unique_ptr<CRope>  m_upRightRope;
    CGameObject*    m_pOwner{};
    CTransform      m_tr;
    CSpringJoint    m_sj;
    CRigidbody      m_rb;

    _bool           m_bDynamicAnchor = false;
    OBJECT_HANDLE   m_hAnchorObject{};
    _float3         m_vLocalAnchorOffset{};

    GAS_STATE               m_tGas{};

    _float3                 m_vAnchor{};
    _float                  m_fRopeMaxDist = 80.f;
    _uint                   m_flagUsingSide = 0;

    CEvent<_uint, _uint>    m_OnSuccessAnchored;

    _bool                   m_bReelBoost = false;

    _float3                 m_vRopeOffset = { 0.f, 0.7f, 0.3f };

private :
    void                    Handle_RopeState(CRope::ROPE_STATE eState, SIDE eSide);
    CGameObject*            Find_Owner();


public :
    /* --- Rope --- */
    void            Try_Grappling(SIDE eSide);
    void            Finish_Grappling(SIDE eSide);

    _bool           Detect_GrapplingPoint(TRY_GRAPPLING_INFO& tInfo);
    _bool           Detect_GrapplingDist(_float* fDist);

    _uint           Get_UsingFlag();
    void            Set_ReelBoost(_bool bEnable);

    _bool           Has_Anchor() const;
    _float3         Get_AnchoredPos() const;

    /* --- Gas --- */
    _bool           Can_UseGas() const;
    void            Fill_Max();
    GAS_STATE       Get_GasState() const;

    /* --- Common ---*/
    void            Bind_PlayerContext(const PLAYER_CONTEXT& tContext);

    template <typename T>
    ListenerID Subscribe_On_Success_Anchored(void(T::* func)(_uint, _uint), T* pInstance)
    {
        return m_OnSuccessAnchored.Add_Listener(func, pInstance);
    }

private :
    _float  m_fForceSpringNormal = 5.f;
    _float  m_fForceDamperNormal = 4.f;
    _float  m_fForceSpringReel = 7.f;
    _float  m_fForceDamperReel = 4.f;

SCRIPT_FIELDS_BEGIN(CODM_Gear)
    SCRIPT_FIELD_FLOAT(m_fForceSpringNormal)
    SCRIPT_FIELD_FLOAT(m_fForceDamperNormal)
    SCRIPT_FIELD_FLOAT(m_fForceSpringReel)
    SCRIPT_FIELD_FLOAT(m_fForceDamperReel)
SCRIPT_FIELDS_END(CODM_Gear)


};

NS_END;
