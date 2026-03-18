#pragma once
#include "Client_Define.h"
#include "Rope.h"
#include "Script.h"
#include "SpringJoint.h"
#include "Transform.h"

#pragma region FD
NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)
class CRope;
NS_END
#pragma endregion

NS_BEGIN(Client)


class CODM_Gear : public IScript
{


private :
    std::unique_ptr<CRope>  m_upRope;
    Engine::CGameObject*    m_pOwner{};
    Engine::CTransform      m_tr;
    Engine::CSpringJoint    m_sj;

    _float3                 m_vAnchor{};
    _float                  m_fSpring = 10.f;
    _float                  m_fDamper = 5.f;

private :
    void            Handle_RopeState(CRope::ROPE_STATE eState);

private :
    void            Is_Anchorable();
    CGameObject*    Find_Owner();

public :
    void Try_Grappling();
    void Finish_Grappling();

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

};

NS_END;
