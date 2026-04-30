#pragma once
#include "Client_Define.h"
#include "Script.h"
#include "CinematicSystem.h"

NS_BEGIN(Client)

class CScout_Controller;

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
        TITAN_BORNE,
        END
    };
    void Enter_State(OPENING_STATE eState);

private:
    void Update_BoatApproach(_float fDT);
    void Update_WhiteOut(_float fDT);
    void Update_TitanBorne(_float fDT);
    void Apply_WhiteOutColor(const _float4& vColor);

    void On_CinematicEvent(const CINEMATIC_EVENT_DATA& tEventData);


private:
    OPENING_STATE       m_eState = OPENING_STATE::NONE;
    CGameObject*        m_pCinematicCam{};
    CCamera             m_camCinematic{};

    _float              m_fStateTime = 0.f;
    _float              m_fOpeningTime = 0.f;

    /* ------------ BOAT_APPROACH ------------ */
    CGameObject*        m_pBoatObject = nullptr;
    CGameObject*        m_pErenObject = nullptr;
    CGameObject*        m_pBoatNPCObject = nullptr;

    CTransform          m_trBoat{};
    CTransform          m_trEren{};
    CTransform          m_trBoatNPC{};

    _float3             m_vBoatStartPos = { -120.f, 6.4f, -210.f};
    _float3             m_vBoatBasePos{};
    _float3             m_vErenLocalOffset = { 0.5f, 8.8f, -2.5f };
    _float3             m_vBoatNPCLocalOffset = { 2.f, 3.f, -1.5f };

    _float              m_fBoatMoveSpeed = 2.5f;
    _float              m_fBoatArriveDist = 0.05f;
    _float              m_fBoatBobAmp = 0.12f;
    _float              m_fBoatBobFreq = 1.35f;

    _float              m_fBoatRollAmp = 4.f;
    _float              m_fBoatRollFreq = 1.1f;

    _float              m_fBoatPitchAmp = 2.f;
    _float              m_fBoatPitchFreq = 1.7f;

    SCRIPT_OBJECT_REF   m_refBoat{};
    SCRIPT_OBJECT_REF   m_refEren{};
    SCRIPT_OBJECT_REF   m_refBoatNPC{};
    _float3             m_vBoatGoalPos{};
    _float3             m_vBoatBaseEuler{};
    _float3             m_vErenBaseEuler{};
    _float3             m_vBoatNPCBaseEuler{};
    /* ------------------------------------------ */


    /* ---------------- WHITEOUT ---------------- */
    CGameObject*        m_pWhiteOutUIObject = nullptr;
    CCanvasRenderer     m_crWhiteOut{};

    _float              m_fWhiteOutFastTime = 0.18f;
    _float              m_fWhiteOutHoldTime = 0.1f;
    _float              m_fWhiteOutFlashTime = 0.12f;
    _float              m_fWhiteOutReturnTime = 0.8f;

    _float4             m_vWhiteColor = { 1.f, 1.f, 1.f, 0.f };
    _float4             m_vYellowFlashColor = { 1.f, 0.92f, 0.55f, 1.f };

    SCRIPT_OBJECT_REF   m_refWhiteOutUI{};
    /* ------------------------------------------ */

    /* --------------- TITAN_BORNE -------------- */
    CGameObject*        m_pErenTitan = nullptr;

    SCRIPT_OBJECT_REF   m_refErenTitan{};

    _float              m_fElapsedBorn = 0.f;
    _bool               m_bRequestedShake = false;
    _bool               m_bErenTitanSpawned = false;
    /* ------------------------------------------ */

    /* --------------- COMMON -------------- */
    CScout_Controller*  m_scScoutController{};

    SCRIPT_OBJECT_REF   m_refCinematicCam{};
    SCRIPT_OBJECT_REF   m_refScoutController{};
    _float              m_fIngameVolume = 0.12f;
    _int                m_iPlayOpening = 0;


private :
SCRIPT_FIELDS_BEGIN(COpening_Director)
    SCRIPT_FIELD_OBJECT_REF(m_refCinematicCam);
    SCRIPT_FIELD_OBJECT_REF(m_refScoutController);
    SCRIPT_FIELD_INT(m_iPlayOpening);

    /* ------------ BOAT_APPROACH ------------ */
    SCRIPT_FIELD_OBJECT_REF(m_refBoat);
    SCRIPT_FIELD_OBJECT_REF(m_refEren);
    SCRIPT_FIELD_OBJECT_REF(m_refBoatNPC);

    SCRIPT_FIELD_FLOAT3(m_vBoatStartPos);
    SCRIPT_FIELD_FLOAT3(m_vBoatGoalPos);
    //SCRIPT_FIELD_FLOAT3(m_vErenLocalOffset);
    //SCRIPT_FIELD_FLOAT3(m_vBoatNPCLocalOffset);

    SCRIPT_FIELD_FLOAT(m_fBoatMoveSpeed);
    /* ------------------------------------------ */

    /* ---------------- WHITEOUT ---------------- */
    SCRIPT_FIELD_OBJECT_REF(m_refWhiteOutUI);
    /* ------------------------------------------ */

    /* -------------- TITAN_BORNE --------------- */
    SCRIPT_FIELD_OBJECT_REF(m_refErenTitan);
    /* ------------------------------------------ */



SCRIPT_FIELDS_END(COpening_Director)
};

NS_END;
