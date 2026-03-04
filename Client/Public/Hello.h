#pragma once
#include <Transform.h>

#include "Client_Define.h"
#include "Script.h"


NS_BEGIN(Client)

class CHello : public IScript
{
public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    CTransform m_Trnasform;

private :
    void Move(_float fDT);
};

NS_END;
