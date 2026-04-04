#pragma once
#include "Client_Define.h"
#include "Rope.h"
#include "Event.h"

#pragma region FD

NS_BEGIN(Client)
class CRope;
typedef struct tagTryGrapplingInfo
{
    _float3 vPoint{};
    _float3 vCamOrigin{};
    _float3 vRayDir{};
    _float  fDist{};
} TYR_GRAPPLING_INFO;

NS_END
#pragma endregion

NS_BEGIN(Client)

class CODM_Gear : public IScript
{
public :
    _float                  m_fSpring = 10.f;
    _float                  m_fDamper = 5.f;

SCRIPT_FIELDS_BEGIN(CODM_Gear)
    SCRIPT_FIELD_FLOAT(m_fSpring)
    SCRIPT_FIELD_FLOAT(m_fDamper)
SCRIPT_FIELDS_END(CODM_Gear)

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    std::unique_ptr<CRope>  m_upLeftRope;
    std::unique_ptr<CRope>  m_upRightRope;
    Engine::CGameObject*    m_pOwner{};
    Engine::CTransform      m_tr;
    Engine::CSpringJoint    m_sj;

    _float3                 m_vAnchor{};

    _float                  m_fRopeMaxDist = 150.f;

    _uint                   m_flagUsingSide = 0;

private :
    void            Handle_RopeState(CRope::ROPE_STATE eState, SIDE eSide);
    CGameObject*    Find_Owner();
    CEvent<_uint, _uint>   m_OnSuccessAnchored;

public :
    void Try_Grappling(SIDE eSide);
    void Finish_Grappling(SIDE eSide);

    _bool           Detect_GrapplingPoint(TYR_GRAPPLING_INFO& tInfo);
    static _bool    Detect_GrapplingDist(_float* fDist);

    _uint           Get_UsingFlag();

    template <typename T>
    ListenerID Subscribe_On_Success_Anchored(void(T::* func)(_uint, _uint), T* pInstance)
    {
        return m_OnSuccessAnchored.Add_Listener(func, pInstance);
    }


};

NS_END;
