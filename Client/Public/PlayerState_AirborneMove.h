#include "PlayerState.h"
#include "Animator.h"

NS_BEGIN(Client)

/* GROUNDED_MOVE, RELOAD, ATTACK */

class CPlayerState_AirborneMove final : public CPlayerState
{
public:
    CPlayerState_AirborneMove(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
    ~CPlayerState_AirborneMove();

public:
    HRESULT Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

    void Enter(_uint iDetailFlag) override;
    void Exit() override;

    void Cache_PlayerContext(const PLAYER_CONTEXT& tContext) override;
    void Setup_CachedPlayerContext() override;

private :
    void Decide_NextState() override;

    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_AirDashFinished(const Engine::ANIMATION_EVENT_DATA& tData);

private :
    AIRBORNE_MOVE           m_eAirborneState = AIRBORNE_MOVE::AIR_BEGIN;
    _float                  m_fGroundStableTime = 0.f;
    _float                  m_fAirStableTime = 0.f;
    _bool                   m_bAirReleasePlayed = false;
    _float                  m_fAirReleaseElapsedTime = 0.f;
    const _float            m_fTotalAirReleaseTime = 2.f;

    class CVFX_Manager*     m_pVFX_Manager = nullptr;
    VFX_OBJECT*             m_pSparkle = nullptr;
    _float                  m_fSlideSparkAcc = 0.f;

    _bool                   m_bContactGround = false;
    _bool                   m_bContactGroundSFXRequested = false;

private:
    void    Decide_HookAnim();

    void    Update_AnchorAirOrSlide();
    void    Decide_AnchorMoveAnim(_bool bOnGround);
    _bool   Is_AnchorSliding() const;
    _bool   Try_AirReleaseMotion();

public:
    static std::shared_ptr<CPlayerState_AirborneMove> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
};



NS_END
