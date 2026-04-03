#pragma once
#include "Client_Define.h"
#include "Scout_Define.h"
#include "Script.h"

NS_BEGIN(Client)

class CScout : public IScript, public CHuman
{
public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    void            Move(_fvector vDir, _float fDT);

private :
    CTransform      m_trOwner{};
    CCollider       m_cldrOwner{};
    CMeshRenderer   m_mrOwner{};
    CAnimator       m_animOwner{};

public :
    void            Set_ActCase();
};

NS_END;
