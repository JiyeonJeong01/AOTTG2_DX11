#pragma once
#include <Transform.h>

#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class CCameraController : public IScript
{
public :
    _float3             m_vOffsetToPlayer = { 0.f, 5.f, -15.f };
    _float              m_fMouseSensor = 0.05f;
    SCRIPT_OBJECT_REF   m_refCamera{};
    SCRIPT_OBJECT_REF   m_refTarget{};

public:
    SCRIPT_FIELDS_BEGIN(CCameraController)
        SCRIPT_FIELD_FLOAT3(m_vOffsetToPlayer)
        SCRIPT_FIELD_FLOAT(m_fMouseSensor)
        SCRIPT_FIELD_OBJECT_REF(m_refCamera)
        SCRIPT_FIELD_OBJECT_REF(m_refTarget)
    SCRIPT_FIELDS_END(CCameraController)

private:
    Engine::CGameObject*    m_goNormalCam{};
    CTransform              m_trNormalCam;

    Engine::CGameObject*    m_goTarget{};
    CTransform              m_trTarget;

private:
    _float  m_fYawDegree = 0.f;
    _float  m_fPitchDegree = 0.f;
    _float  m_fDistance = 0.f;
    _float  m_fHeight = 0.f;

private:
    void Follow_Target();

public :
    void Pitch(_float fDegree);
    void Yaw(_float fDegree);

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;
};

NS_END;
