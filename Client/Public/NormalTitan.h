#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)

class CNormalTitan : public IScript
{
public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;
};

NS_END;
