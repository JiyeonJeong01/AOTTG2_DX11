#pragma once
#include "PlayerState.h"

class CPlayerState_Grabbed final : public CPlayerState
{
public:
    CPlayerState_Grabbed(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
    virtual ~CPlayerState_Grabbed();

public:
    virtual HRESULT Initialize() override;

    virtual void Setup_CachedPlayerContext() override;

    virtual void Priority_Update(_float fDT) override;
    virtual void Update(_float fDT) override;
    virtual void Late_Update(_float fDT) override;

    virtual void Enter(_uint iDetailFlag = 0) override;
    virtual void Exit() override;

private:
    void Decide_NextState();
    void Start_DeathFade();
    void Update_DeathFade(_float fDT);

    _float m_fElapsedGrabTime = 0.f;
    _float m_fFadeTime = 0.f;
    _float m_fFadeDuration = 3.f;
    _bool m_bFadeOut = false;

    Engine::CGameObject* m_goFadeUI = nullptr;
    Engine::CCanvasRenderer m_crFadeUI{};

public:
    static std::shared_ptr<CPlayerState_Grabbed> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
};
