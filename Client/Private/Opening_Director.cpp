#include "Opening_Director.h"
#include "GameInstance.h"
#include "Easing_Function.h"
#include "AnimationClip_Eren.h"
#include "Scout_Controller.h"

NS_BEGIN(Client)

void COpening_Director::Awake(void* pCtx)
{
    CGameObject* pObject = nullptr;

    {
        pObject = GAME_INSTANCE.Find_GameObject(m_refCinematicCam.hObject);
        if (pObject != nullptr)
        {
            m_pCinematicCam = pObject;
            m_camCinematic = m_pCinematicCam->Get_Component<CCamera>();
        }

        pObject = GAME_INSTANCE.Find_GameObject(m_refScoutController.hObject);
        if (pObject != nullptr)
        {
            m_scScoutController = pObject->Get_Script<CScout_Controller>();
        }
    }

    {
        pObject = GAME_INSTANCE.Find_GameObject(m_refBoat.hObject);
        if (pObject != nullptr)
        {
            m_pBoatObject = pObject;
            m_trBoat = m_pBoatObject->Get_Component<CTransform>();
        }

        pObject = GAME_INSTANCE.Find_GameObject(m_refEren.hObject);
        if (pObject != nullptr)
        {
            m_pErenObject = pObject;
            m_trEren = m_pErenObject->Get_Component<CTransform>();
        }

        pObject = GAME_INSTANCE.Find_GameObject(m_refBoatNPC.hObject);
        if (pObject != nullptr)
        {
            m_pBoatNPCObject = pObject;
            m_trBoatNPC = m_pBoatNPCObject->Get_Component<CTransform>();
        }
    }

    {
        pObject = GAME_INSTANCE.Find_GameObject(m_refWhiteOutUI.hObject);

        if (pObject != nullptr)
        {
            m_pWhiteOutUIObject = pObject;
            m_crWhiteOut = m_pWhiteOutUIObject->Get_Component<CCanvasRenderer>();
        }
    }

    {
        pObject = GAME_INSTANCE.Find_GameObject(m_refErenTitan.hObject);

        if (pObject != nullptr)
        {
            m_pErenTitan = pObject;
            m_pErenTitan->Set_Enable(false);
        }
    }
}

void COpening_Director::Start(void* pCtx)
{
    if (g_bSkipOpeningOnce)
    {
        g_bSkipOpeningOnce = false;
        Enter_State(OPENING_STATE::TITAN_BORNE);
        SYS_SOUND.PlayBGM(L"InGameBGM", m_fIngameVolume);
        return;
    }

    if (m_iPlayOpening)
    {
        if (m_trBoat.Is_Valid())
        {
            m_trBoat.Set_Position(XMLoadFloat3(&m_vBoatStartPos));
            m_vBoatBasePos = m_vBoatStartPos;

            m_vBoatBaseEuler = m_trBoat.Get_Rotation_Euler();
            m_vErenBaseEuler = m_trEren.Get_Rotation_Euler();
            m_vBoatNPCBaseEuler = m_trBoatNPC.Get_Rotation_Euler();
        }
    
        if (m_crWhiteOut.Is_Valid())
        {
            m_crWhiteOut.Set_Color(_float4(1.f, 1.f, 1.f, 0.f));
        }

        SYS_CINEMATIC.Subscribe_CinematicEvent(
            "opening",
            CINEMATIC_EVENT_TYPE::CUSTOM,
            &COpening_Director::On_CinematicEvent,
            this);


        SYS_CINEMATIC.Play("opening", m_camCinematic);

        Enter_State(OPENING_STATE::BOAT_APPROACH);
        SYS_SOUND.PlayBGM(L"OpeningCutScene_TitanReveal", 0.1f);
        SYS_SOUND.PlayForceSFX(L"OpeningCutScene_Boat", CHANNEL_26, 0.3f);
    }
    else
    {
        Enter_State(OPENING_STATE::TITAN_BORNE);
        SYS_SOUND.PlayBGM(L"InGameBGM", m_fIngameVolume);
    }
}

void COpening_Director::Priority_Update(void* pCtx, _float fDT)
{
}

void COpening_Director::Update(void* pCtx, _float fDT)
{
    if (!m_trBoat.Is_Valid())
        return;

    m_fOpeningTime += fDT;
    m_fStateTime += fDT;

    switch (m_eState)
    {
    case OPENING_STATE::BOAT_APPROACH:
        Update_BoatApproach(fDT);
        break;

    case OPENING_STATE::BOAT_ARRIVED_WAIT:
            Enter_State(OPENING_STATE::WHITEOUT);
        break;

    case OPENING_STATE::WHITEOUT:
        /* 나중에 UI 화이트 패널 alpha 증가 */
        Update_WhiteOut(fDT);
        break;

    case OPENING_STATE::TITAN_BORNE:
        Update_TitanBorne(fDT);
        break;
    }
}

void COpening_Director::Late_Update(void* pCtx, _float fDT)
{
}

void COpening_Director::Enter_State(OPENING_STATE eState)
{
    m_eState = eState;
    m_fStateTime = 0.f;

    switch (m_eState)
    {
    case OPENING_STATE::BOAT_APPROACH:
        break;

    case OPENING_STATE::WHITEOUT:
    {
        m_bErenTitanSpawned = false;

        if (m_pErenObject)
            m_pErenObject->Set_Enable(false);
        if (m_pBoatNPCObject)
            m_pBoatNPCObject->Set_Enable(false);
    }
    break;
    case OPENING_STATE::TITAN_BORNE:
        m_fElapsedBorn = 0.f;
        if (m_pErenTitan)
        {
            m_pErenTitan->Set_Enable(true);
        }
        break;
    }
}

void COpening_Director::Update_BoatApproach(_float fDT)
{
    _float3 vCurBasePos = m_vBoatBasePos;

    _float3 vToGoal =
    {
        m_vBoatGoalPos.x - vCurBasePos.x,
        m_vBoatGoalPos.y - vCurBasePos.y,
        m_vBoatGoalPos.z - vCurBasePos.z
    };

    const _float fDistSq =
        vToGoal.x * vToGoal.x +
        vToGoal.y * vToGoal.y +
        vToGoal.z * vToGoal.z;

    /* 도착 */
    if (fDistSq <= (m_fBoatArriveDist * m_fBoatArriveDist))
    {
        m_vBoatGoalPos = m_vBoatBasePos;
    }
    else
    {
        const _float fDist = sqrtf(fDistSq);

        _float3 vDir =
        {
            vToGoal.x / fDist,
            vToGoal.y / fDist,
            vToGoal.z / fDist
        };

        /* 일단 이동 */
        m_vBoatBasePos.x += vDir.x * m_fBoatMoveSpeed * fDT;
        m_vBoatBasePos.y += vDir.y * m_fBoatMoveSpeed * fDT;
        m_vBoatBasePos.z += vDir.z * m_fBoatMoveSpeed * fDT;
    }

    _float3 vFinalPos = m_vBoatBasePos;
    vFinalPos.y += sinf(m_fOpeningTime * m_fBoatBobFreq) * m_fBoatBobAmp;

    m_trBoat.Set_Position(XMLoadFloat3(&vFinalPos));

    /* 값 : 삼각함수 (시간 * 주파수) * 세기 */
    _float3 vBoatEuler = m_vBoatBaseEuler;

    _float fDeltaX = cosf(m_fOpeningTime * m_fBoatPitchFreq) * m_fBoatBobAmp;
    _float fDeltaY = sinf(m_fOpeningTime * m_fBoatRollFreq) * m_fBoatRollAmp;

    vBoatEuler.x += fDeltaX;
    vBoatEuler.z += fDeltaY;

    m_trBoat.Set_Rotation_Euler(vBoatEuler);

    _matrix matBoatWorld = m_trBoat.Get_WorldXM();

    _vector vRight = XMVector3Normalize(matBoatWorld.r[0]);
    _vector vUp = XMVector3Normalize(matBoatWorld.r[1]);
    _vector vLook = XMVector3Normalize(matBoatWorld.r[2]);
    _vector vPos = matBoatWorld.r[3];

    _vector vErenWorld =
        vPos +
        vRight * m_vErenLocalOffset.x +
        vUp * m_vErenLocalOffset.y +
        vLook * m_vErenLocalOffset.z;

    _vector vBoatNPCWorld =
        vPos +
        vRight * m_vBoatNPCLocalOffset.x +
        vUp * m_vBoatNPCLocalOffset.y +
        vLook * m_vBoatNPCLocalOffset.z;

    m_trEren.Set_Position(vErenWorld);
    m_trBoatNPC.Set_Position(vBoatNPCWorld);

    if (fDistSq <= (m_fBoatArriveDist * m_fBoatArriveDist))
    {
        Enter_State(OPENING_STATE::BOAT_ARRIVED_WAIT);
    }
}

void COpening_Director::Update_WhiteOut(_float fDT)
{
    if (!m_crWhiteOut.Is_Valid())
        return;

    const _float fTime = m_fStateTime;

    const _float fFastEnd = m_fWhiteOutFastTime;
    const _float fHoldEnd = fFastEnd + m_fWhiteOutHoldTime;
    const _float fFlashEnd = fHoldEnd + m_fWhiteOutFlashTime;
    const _float fReturnEnd = fFlashEnd + m_fWhiteOutReturnTime;

    if (!m_bErenTitanSpawned && fTime >= fFastEnd * 0.98f)
    {
        m_bErenTitanSpawned = true;

        if (m_pErenTitan)
            m_pErenTitan->Set_Enable(true);

        m_fElapsedBorn = 0.f;
    }

    _float4 vColor = { 1.f, 1.f, 1.f, 0.f };

    if (fTime <= fFastEnd)
    {
        const _float t = CEasingFunction::Clamp01(fTime / m_fWhiteOutFastTime);

        vColor.x = 1.f;
        vColor.y = 1.f;
        vColor.z = 1.f;
        vColor.w = CEasingFunction::Lerp(0.f, 1.f, t);
    }
    else if (fTime <= fHoldEnd)
    {
        vColor = _float4(1.f, 1.f, 1.f, 1.f);
    }
    else if (fTime <= fFlashEnd)
    {
        const _float t = CEasingFunction::Clamp01((fTime - fHoldEnd) / m_fWhiteOutFlashTime);

        /* 흰색 -> 노란색 -> 흰색 빠르게 깜빡 */
        _float fPulse = sinf(t * 3.141592f);

        vColor.x = CEasingFunction::Lerp(1.f, m_vYellowFlashColor.x, fPulse);
        vColor.y = CEasingFunction::Lerp(1.f, m_vYellowFlashColor.y, fPulse);
        vColor.z = CEasingFunction::Lerp(1.f, m_vYellowFlashColor.z, fPulse);
        vColor.w = 1.f;
    }
    else if (fTime <= fReturnEnd)
    {
        const _float t = CEasingFunction::Clamp01((fTime - fFlashEnd) / m_fWhiteOutReturnTime);

        /* 천천히 다시 투명으로 */
        vColor.w = CEasingFunction::Lerp(1.f, 0.f, t);
    }
    else
    {
        vColor = _float4(1.f, 1.f, 1.f, 0.f);

        Enter_State(OPENING_STATE::TITAN_BORNE);
    }

    Apply_WhiteOutColor(vColor);
}

void COpening_Director::Update_TitanBorne(_float fDT)
{
    if (m_bRequestedShake)
        return;
    m_fElapsedBorn += fDT;
    if (!m_bRequestedShake && m_fElapsedBorn > 0.4f)
    {
        SYS_CINEMATIC.Force_Shake(1.2f, 0.15f);
        m_bRequestedShake = true;
    }
}

void COpening_Director::Apply_WhiteOutColor(const _float4& vColor)
{
    if (!m_crWhiteOut.Is_Valid())
        return;

    m_crWhiteOut.Set_Color(vColor);
}

void COpening_Director::On_CinematicEvent(const CINEMATIC_EVENT_DATA& tEventData)
{
    if (tEventData.strEventName == "TOGATE")
    {
        SYS_SOUND.StopSound(CHANNEL_26);
    }
    else if (tEventData.strEventName == "SCOUTS")
    {
        SYS_SOUND.PlayBGM(L"InGameBGM", 0.3f);
    }
    else if (tEventData.strEventName == "ORBIT")
    {
        if (m_scScoutController)
            m_scScoutController->Play_Salute_Animation();
        else
            LOG_ERROR("m_scScoutController is nullptr!");
    }
    else if (tEventData.strEventName == "GRUNT")
    {
        SYS_SOUND.PlayForceSFX(L"Titan_Grunt3", CHANNEL_27, 0.8f);
    }
    else if (tEventData.strEventName == "WHITEOUT")
    {
        if (m_eState != OPENING_STATE::WHITEOUT &&
            m_eState != OPENING_STATE::TITAN_BORNE &&
            m_eState != OPENING_STATE::END)
        {
            Enter_State(OPENING_STATE::WHITEOUT);

            if (m_scScoutController)
                m_scScoutController->Enable_Object(false);
            else
                LOG_ERROR("m_scScoutController is nullptr!");

            SYS_SOUND.SetChannelVolume(CHANNEL_0, m_fIngameVolume);
            SYS_SOUND.PlayForceSFX(L"OpeningCutScene_Lighting", CHANNEL_27, 0.8f);
        }

    }
    else if (tEventData.strEventName == "FORCEEXIT")
    {
        m_camCinematic.Set_Priority(0);
    }
}

NS_END
