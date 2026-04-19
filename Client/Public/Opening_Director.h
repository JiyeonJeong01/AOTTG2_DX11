#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)

class COpening_Director : public IScript
{
public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;
    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private:
    enum class OPENING_STATE : uint32_t
    {
        NONE,
        BOAT_APPROACH,
        BOAT_ARRIVED_WAIT,
        WHITEOUT,
        TITAN_REVEAL,
        END
    };

private:
    void Update_BoatApproach(_float fDT);
    void Enter_State(OPENING_STATE eState);

private:
    SCRIPT_OBJECT_REF   m_refBoat{};
    _float3             m_vBoatGoalPos{};
    _float3             m_vBoatBaseEuler{};
private:
    CGameObject* m_pBoatObject = nullptr;
    CTransform          m_trBoat{};

private:
    OPENING_STATE       m_eState = OPENING_STATE::NONE;
    _float              m_fStateTime = 0.f;
    _float              m_fOpeningTime = 0.f;

private:
    _float3             m_vBoatStartPos{};
    _float3             m_vBoatBasePos{};

private:
    _float              m_fBoatMoveSpeed = 2.5f;
    _float              m_fBoatArriveDist = 0.25f;
    _float              m_fBoatBobAmp = 0.12f;
    _float              m_fBoatBobFreq = 1.35f;

    _float              m_fBoatRollAmp = 4.f;
    _float              m_fBoatRollFreq = 1.1f;

    _float              m_fBoatPitchAmp = 2.f;
    _float              m_fBoatPitchFreq = 1.7f;

private :
SCRIPT_FIELDS_BEGIN(COpening_Director)
    SCRIPT_FIELD_OBJECT_REF(m_refBoat);
    SCRIPT_FIELD_FLOAT3(m_vBoatGoalPos);

    SCRIPT_FIELD_FLOAT(m_fBoatMoveSpeed);
    SCRIPT_FIELD_FLOAT(m_fBoatArriveDist);

SCRIPT_FIELDS_END(COpening_Director)
};

NS_END;
