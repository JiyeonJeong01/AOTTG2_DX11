#pragma once

#include "Client_Define.h"

NS_BEGIN(Client)

class CThrownBlade final : public IScript
{
public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

public:
    void Start_Throw(_fvector vStartPoint, _fvector vDir);


private:
    CGameObject*         m_pThrownBlade{};

    CTransform           m_trBlade;

    _float3              m_vStartPoint{};
    _float3              m_vDir{};

    _bool                m_bStarted = false;

    _float               m_fRotPerSec = 540.f;
    _float               m_fThrowSpeed = 40.f;
    const std::string    m_strBlade = "ThrownBlade";
private:
    void On_TriggerEnter(const COLLISION_DESC& tDesc);

};


NS_END
