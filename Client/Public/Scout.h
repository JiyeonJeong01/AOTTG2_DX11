#pragma once
#include "Client_Define.h"
#include "Scout_Define.h"
#include "Script.h"

NS_BEGIN(Client)

class CScoutBehavior;
class CScout_Scriptable_Object;

class CScout : public IScript, public CHuman
{
public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private:
    void            Move(_fvector vDir, _float fDT);
    void            SetUp_Context();
    void            SetUp_Behavior();
    void            Clear_Behavior();

public:
    void            Set_ActCase();
    void            On_Grabbed(SIDE eSide, CTitan* pTitan) override;

private:
    CTransform      m_trOwner{};
    CCollider       m_cldrOwner{};
    CMeshRenderer   m_mrOwner{};
    CAnimator       m_animOwner{};

    CGameObject*    m_goOwner = nullptr;

    SCOUT_STATS     m_tStats{};
    SCOUT_CONTEXT   m_tContext{};

    CScoutBehavior* m_pBehavior = nullptr;
};

NS_END
