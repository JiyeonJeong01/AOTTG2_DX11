#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)

class CBoxHotReload : public IScript
{
public:
    void Awake(void* pCtx);
    void Start(void* pCtx);

    void Priority_Update(void* pCtx, _float fDT);
    void Update(void* pCtx, _float fDT);
    void Late_Update(void* pCtx, _float fDT);
};

NS_END;
