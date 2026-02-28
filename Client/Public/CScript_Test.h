#pragma once

#include "Client_Define.h"
#include "Script.h"
#include "Transform.h"

NS_BEGIN(Client)

class CScript_Test : public IScript
{
public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    CTransform m_transform{};

    _float m_fSpeed{ };

private :
    void Handle_Input(_float fDT);



};

NS_END;
