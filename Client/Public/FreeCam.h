#pragma once
#include "Client_Define.h"
#include "Script.h"
#include "Transform.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class CFreeCam : public IScript
{

public :
    _float              m_fSpeed = 10.f;
    _float              m_fMouseSense = 0.5f;
    SCRIPT_OBJECT_REF   m_roCamera;

    SCRIPT_FIELDS_BEGIN(CFreeCam)
        SCRIPT_FIELD_FLOAT(m_fSpeed)
        SCRIPT_FIELD_FLOAT(m_fMouseSense)
        SCRIPT_FIELD_OBJECT_REF(m_roCamera)
    SCRIPT_FIELDS_END(CFreeCam)

private :
    CTransform      m_Transform;
    CGameObject*    m_goFreeCam = nullptr;

private :
    CGameObject* Find_FreeCam();

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;
};

NS_END;
