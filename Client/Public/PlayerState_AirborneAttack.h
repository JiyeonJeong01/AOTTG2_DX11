#include "PlayerState.h"
#include "Animator.h"

NS_BEGIN(Client)

class CThrownBlade;

class CPlayerState_AirborneAttack final : public CPlayerState
{
public:
    CPlayerState_AirborneAttack(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
    ~CPlayerState_AirborneAttack();

public:
    HRESULT Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

    void Enter(_uint iDetailFlag) override;
    void Exit() override;

    void Cache_PlayerContext(const PLAYER_CONTEXT& tContext) override;
    void Setup_CachedPlayerContext() override;

private:
    void Decide_NextState() override;

    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_NomalFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_SpinH_Finished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_Throw_Finished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_SpinV_Finished(const Engine::ANIMATION_EVENT_DATA& tData);
private:
    AIRBORNE_ATTACK     m_eAirborneAttackState = AIRBORNE_ATTACK::END;
    CHitBox*            m_pHitBox = nullptr;

    _bool               m_bAnimFinished = false;
    _bool               m_bKeepAttack = false;

    _float              m_fOriginAnimPlaySpeed = 1.f;

    /* SPIN_H */
    _bool               m_bSpinH_Completed = false;
    _bool               m_bSpinH_Looping = false;
    _int                m_iSpinH_LoopCnt = 0;
    const _float        m_fSpinH_LoopStart_TrackPosition = 29.f;
    const _float        m_fSpinH_LoopEnd_TrackPosition = 46.f;
    const _int          m_iSpinH_TotalLoopCnt = 2;
    const _float        m_fSpinH_Speed = 2.3f;

    /* THROW */
    CThrownBlade*       m_pThrownBlade{};
    _bool               m_bThrowNow = false;
    _bool               m_bThrewAlready= false;
    _float              m_fThrow_WaitElapsedTime = 0.f;
    _float              m_fThrow_WaitTotalTime = 0.1f;
    _float3             m_vThrowOffset = { 0.f, 1.f, 1.f };


    /* SPIN_V */
    _bool               m_bSpinV_Completed = false;
    _bool               m_bSpinV_Looping = false;
    _int                m_iSpinV_LoopCnt = 0;
    const _int          m_iSpinV_TotalLoopCnt = 2;
    const _float        m_fSpinV_LoopStart_TrackPosition = 20.f;
    const _float        m_fSpinV_LoopEnd_TrackPosition = 28.f;


    CHitBox*            m_pBladeHitBox = nullptr;

    _float              m_fNormal_HitBoxStartTrackPos = 0.f;
    _float              m_fSpinH_HitBoxStartTrackPos = 0.f;
    _float              m_fSpinV_HitBoxStartTrackPos = 0.f;

    _bool               m_bBladeHitBoxStarted = false;

private :
    void    Update_BladeHitBox(_float fDT);
    void    Spin_Horizontal(_float fDT);
    void    Spin_Vertical(_float fDT);
    void    Throw_Blade(_float fDT);

    void    Set_InitialValue();

    void    Decide_State_If_Needed();
    _bool   Can_Start_BladeHitBox() const;
    _bool   Can_End_BladeHitBox() const;
public:
    static std::shared_ptr<CPlayerState_AirborneAttack> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
};



NS_END
