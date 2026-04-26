#pragma once

#include "Client_Define.h"
#include "Identity.h"
#include "BuiltIn_GUID.h"

NS_BEGIN(Client)

enum class TITAN_TYPE { NONE, NORMAL, ABNORMAL, CRAWLER, END };

class CTitan;

class CHuman
{
public:
    virtual ~CHuman() = default;
    virtual void On_Grabbed(SIDE eSide, CTitan* pTitan) = 0;
    virtual void On_Dead() {};

protected :
    ENTITY_VOLUME   tVolume{};
};

class CTitan
{
public :
    virtual ~CTitan() = default;
    virtual void On_Grab(SIDE eSide, CHuman* pHuman) = 0;
    virtual void On_Dead(const _float fAccuracy) {};
    virtual void On_Stunned(const HIT_INFO& tHitInfo) {};
    virtual TITAN_TYPE Get_TitanType() const = 0;

    _bool           Is_Alive() const {
        return m_bAlive;
    }
    void            Set_Dead() {
        m_bAlive = false;
    }

    virtual _bool   Is_Moving() = 0;
    _bool   Is_FootStep() const
    {
        return m_bFootStep;
    }

protected :
    _bool   m_bAlive = true;
    _bool   m_bFootStep = false;

    _bool   m_bDissolveStarted = false;
    _float  m_fDissolveAmount = -0.15f;
    _float  m_fDissolveSpeed = 0.16f;
    uint32_t m_hDissolveNoiseMap = INVALID_HANDLE_UINT;
    uint32_t m_hDissolveNoiseMeshMap = INVALID_HANDLE_UINT;
    ASSET_GUID  m_tDissolveGUID = ASSET_GUID("D489136C-2D80-4807-BFB8-3E0629218123");
    ASSET_GUID  m_tDissolveMeshGUID = Engine::DefaultAssetGuid::MATERIAL_OUTLINE;
};



NS_END
