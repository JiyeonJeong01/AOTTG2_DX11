#pragma once
#include "ScoutBehavior.h"

NS_BEGIN(Client)

class CPlayer;

class CScoutBehavior_RequestResupply final : public CScoutBehavior
{
public:
    CScoutBehavior_RequestResupply(Engine::CGameObject* goScout, CScout* scScout, SCOUT_BEHAVIOR eBehavior);
    ~CScoutBehavior_RequestResupply() override;

public:
    void Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

public:
    void Process_Start();     // 외부에서 호출

private:
    HRESULT SetUp_References();

    void OnTriggerEnter(const COLLISION_DESC& tCollisionDesc);
    void OnTriggerExit(const COLLISION_DESC& tCollisionDesc);

    _bool Is_Player(const CGameObject* pOther) const;
    void On_DetectedPlayer(_bool bDetected, CGameObject* pPlayer);

private:
    void Process_DetectPlayer(_float fDT);
    void Process_MoveBehindPlayer(_float fDT);
    void Process_Resupply();
    void Process_Finish(_float fDT);

    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);

private:
    enum class RESUPPLY_STATE
    {
        NONE,
        DETECT,
        MOVE,
        RESUPPLY,
        FINISH
    };

private:
    CMeshRenderer   m_mr;

    RESUPPLY_STATE  m_eState = RESUPPLY_STATE::NONE;

    CGameObject*    m_goDetectedPlayer = nullptr;
    _bool           m_bPlayerDetected = false;

    _float          m_fOutlineWidth = 5.f;

    _float          m_fStopAnimationDist = 2.5f;
    _float          m_fBehindDistance = 1.f;
    _float          m_fResupplyDistance = 0.8f;

    _float3         m_vExitPos{ 0.f, 0.f, 0.f };

    _bool           m_bAnimFinished = false;
};

NS_END
