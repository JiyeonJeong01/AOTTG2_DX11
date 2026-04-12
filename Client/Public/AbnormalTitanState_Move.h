#pragma once

#include "TitanState.h"

NS_BEGIN(Client)

class CAbnormalTitanState_Move final : public CTitanState
{
public:
    CAbnormalTitanState_Move(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CAbnormalTitanState_Move();

public:
    HRESULT Initialize() override;
    void    Priority_Update(_float fDT) override;
    void    Update(_float fDT) override;
    void    Late_Update(_float fDT) override;

    void    Enter(_uint iDetailFlag) override;
    void    Exit() override;

    void    Cache_TitanContext(const TITAN_CONTEXT& tContext) override;
    void    Setup_CachedTitanContext() override;

    _uint   Get_DetailState() const override;

private:
    void    Decide_NextState() override;
    void    Decide_NextAnim() override;

private:
    TITAN_MOVE  m_eMoveState = TITAN_MOVE::WALK;

    _float      m_fElapsedMoveTime = 0.f;
    _float      m_fMaxMoveTime = 8.f;
    _float3     m_vPatrolPos = {};

    _char       m_szMoveAnimName[32];

    _float          m_fOriginalRotationSharpness = 0.f;
    _float          m_fFastRotationSharpness = 3.f;

public:
    static std::shared_ptr<CAbnormalTitanState_Move> Create(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
};

NS_END
