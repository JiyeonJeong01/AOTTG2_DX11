#include "PlayerState.h"
#include "Animator.h"

NS_BEGIN(Client)

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

    void Setup_CachedPlayerInfos() override;

private:
    void Decide_NextState() override;

    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_NomalFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_SpinH_Finished(const Engine::ANIMATION_EVENT_DATA& tData);

private:
    AIRBORNE_ATTACK     m_eAirborneAttackState = AIRBORNE_ATTACK::END;

    _bool               m_bAnimFinished = false;
    _bool               m_bKeepAttack = false;

    /* SPIN_H */
    _bool               m_bSpinH_Force = false;
    _float              m_SpinH_Elapsed_Degree = 0.f;
    const _float        m_SpinH_Total_Degree = 1440.f;
    const _float        m_SpinH_Degree_PerSec = 1440.f;

    _float              m_fSpinH_WaitElapsedTime = 0.f;
    _float              m_fSpinH_WaitTotalTime = 0.1f;

    void Spin_Horizontal(_float fDT);
    void Spin_Vertical(_float fDT);
    void Throw_Blade(_float fDT);

    void Set_InitialValue();

    void Decide_State_If_Needed();
public:
    static std::shared_ptr<CPlayerState_AirborneAttack> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
};



NS_END
