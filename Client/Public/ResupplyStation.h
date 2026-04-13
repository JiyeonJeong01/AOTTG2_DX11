#pragma once
#include "Client_Define.h"

NS_BEGIN(Client)

class CResupplyStation final : public IScript
{
public:
    CResupplyStation();
    ~CResupplyStation() override;

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

public:
    _bool Is_PlayerDetected() const
    {
        return m_bPlayerDetected;
    }
    CGameObject* Get_DetectedPlayer() const
    {
        return m_goDetectedPlayer;
    }
    _float3 Get_UIWorldPosition() const;

public:
    void OnTriggerEnter(const COLLISION_DESC& tCollisionDesc);
    void OnTriggerExit(const COLLISION_DESC& tCollisionDesc);

private:
    _bool Is_Player(const CGameObject* pOther) const;
    void On_DetectedPlayer(_bool bDetected, CGameObject* pPlayer);

private:

    SCRIPT_OBJECT_REF m_refTriggerObject;
    SCRIPT_OBJECT_REF m_refUIPivotObject;


private:
    CGameObject* m_goOwner = nullptr;
    CGameObject* m_goTriggerObject = nullptr;
    CGameObject* m_goUIPivotObject = nullptr;
    CGameObject* m_goDetectedPlayer = nullptr;

    CTransform        m_trOwner{};
    CTransform        m_trTrigger{};
    CTransform        m_trUIPivot{};
    CCollider         m_clTrigger{};

    _bool             m_bPlayerDetected = false;

SCRIPT_FIELDS_BEGIN(CResupplyStation)
    SCRIPT_FIELD_OBJECT_REF(m_refTriggerObject)
    SCRIPT_FIELD_OBJECT_REF(m_refUIPivotObject)
SCRIPT_FIELDS_END(CResupplyStation)
};

NS_END
