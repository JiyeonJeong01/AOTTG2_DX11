#include "ScoutBehaviour_RescueDialogue.h"

#include "AnimationClip_Player.h"

#include "Easing_Function.h"
#include "CinematicSystem.h"
#include "UI_NoticeController.h"

NS_BEGIN(Client)

CScoutBehavior_RescueDialogue::CScoutBehavior_RescueDialogue(
    Engine::CGameObject* goScout,
    CScout* scScout,
    SCOUT_BEHAVIOR eBehavior)
    : CScoutBehavior(goScout, scScout, eBehavior)
{
}

CScoutBehavior_RescueDialogue::~CScoutBehavior_RescueDialogue()
{
    if (m_scStagingCamera.Is_Valid())
        m_scStagingCamera.Set_Priority(0);

    if (m_goStagingCamera)
        m_goStagingCamera->Set_Enable(false);
}

void CScoutBehavior_RescueDialogue::Initialize()
{
    m_goTargetPlayer = nullptr;

    m_eState = RESCUE_DIALOGUE_STATE::NONE;
    m_eDialogueState = DIALOGUE_STATE::NONE;
    m_eStagingCameraState = STAGING_CAMERA_STATE::NONE;

    m_fStagingCameraLerpTime = 0.f;

    m_iCurDialogueLine = 0;
    m_iCurDialogueChar = 0;
    m_fDialogueCharTime = 0.f;
    m_fDialogueWaitTime = 0.f;
    m_fFadeAlpha = 0.f;
    m_fFadeTime = 0.f;

    m_vecDialogueLines.clear();
    m_vecDialogueLines.emplace_back(L"정신을 차렸구나!");
    m_vecDialogueLines.emplace_back(L"다행히 먹히기 전에 구할 수 있었어.");
    m_vecDialogueLines.emplace_back(L"아까 구해준 목숨 값이야!");
    m_vecDialogueLines.emplace_back(L"탈환 후 꼭 다시 보자!");

    if (m_tComponents.animator.Is_Valid())
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::IDLE_CASUAL_M);
}

void CScoutBehavior_RescueDialogue::Priority_Update(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);
}

void CScoutBehavior_RescueDialogue::Update(_float fDT)
{
    switch (m_eState)
    {
    case RESCUE_DIALOGUE_STATE::NONE:
        Process_None(fDT);
        return;

    case RESCUE_DIALOGUE_STATE::MOVE_TO_PLAYER:
        Process_MoveToPlayer(fDT);
        return;

    case RESCUE_DIALOGUE_STATE::DIALOGUE:
        Process_Dialogue(fDT);
        return;

    case RESCUE_DIALOGUE_STATE::EXIT:
        Process_Exit(fDT);
        return;
    }
}

void CScoutBehavior_RescueDialogue::Late_Update(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);
}

void CScoutBehavior_RescueDialogue::Process_Start(CGameObject* pPlayer)
{
    m_goTargetPlayer = pPlayer;

    if (m_goScout)
        m_goScout->Set_Enable(true);

    if (m_tComponents.meshRenderer.Is_Valid())
        m_tComponents.meshRenderer.Set_Enable(true);

    Change_State(RESCUE_DIALOGUE_STATE::NONE);

    SYS_CINEMATIC.Subscribe_CinematicEvent(
        "Scout_RescueDialogue",
        CINEMATIC_EVENT_TYPE::CUSTOM,
        &CScoutBehavior_RescueDialogue::On_CinematicEvent,
        this);

    if (m_scCinematicCamera.Is_Valid())
        DEBUG_POINT;

    SYS_CINEMATIC.Play("Scout_RescueDialogue", m_scCinematicCamera);
}

void CScoutBehavior_RescueDialogue::Set_Cameras(CGameObject* pStagingCamera, CGameObject* pCinematicCamera)
{
    m_goStagingCamera = pStagingCamera;
    m_scStagingCamera = {};
    m_trStagingCamera = {};

    if (m_goStagingCamera)
    {
        m_scStagingCamera = m_goStagingCamera->Get_Component<CCamera>();
        m_trStagingCamera = m_goStagingCamera->Get_Component<CTransform>();

        IF_TRUE_RETURN_MSG_BREAK(!m_scStagingCamera.Is_Valid(), , "staging camera component is invalid");
        IF_TRUE_RETURN_MSG_BREAK(!m_trStagingCamera.Is_Valid(), , "staging camera transform is invalid");

        m_goStagingCamera->Set_Enable(false);
        m_scStagingCamera.Set_Priority(0);
    }

    m_goCinematicCamera = pCinematicCamera;
    m_scCinematicCamera = {};

    if (m_goCinematicCamera)
    {
        m_scCinematicCamera = m_goCinematicCamera->Get_Component<CCamera>();
        IF_TRUE_RETURN_MSG_BREAK(!m_scCinematicCamera.Is_Valid(), , "cinematic camera component is invalid");
    }

    if (m_scCinematicCamera.Is_Valid())
        DEBUG_POINT;
}

void CScoutBehavior_RescueDialogue::Set_UI(CUI_NoticeController* pNotice, CGameObject* pDialogue, CGameObject* pFade)
{
    m_pNotice = pNotice;

    m_goDialogueUI = pDialogue;
    m_crDialoguePanel = {};
    m_txtDialogue = {};

    if (m_goDialogueUI)
    {
        m_crDialoguePanel = m_goDialogueUI->Get_Component<CCanvasRenderer>();
        m_txtDialogue = m_goDialogueUI->Get_Component<CUIText>();

        IF_TRUE_RETURN_MSG_BREAK(!m_crDialoguePanel.Is_Valid(), , "dialogue panel canvas renderer is invalid");
        IF_TRUE_RETURN_MSG_BREAK(!m_txtDialogue.Is_Valid(), , "dialogue text is invalid");

        m_goDialogueUI->Set_Enable(false);
    }

    m_goFadeUI = pFade;
    m_crFadeUI = {};

    if (m_goFadeUI)
    {
        m_crFadeUI = m_goFadeUI->Get_Component<CCanvasRenderer>();
        IF_TRUE_RETURN_MSG_BREAK(!m_crFadeUI.Is_Valid(), , "fade ui canvas renderer is invalid");

        m_goFadeUI->Set_Enable(false);

        _float4 vColor = { 0.f, 0.f, 0.f, 0.f };
        m_crFadeUI.Set_Color(vColor);
    }
}

void CScoutBehavior_RescueDialogue::Change_State(RESCUE_DIALOGUE_STATE eState)
{
    m_eState = eState;

    if (m_scCinematicCamera.Is_Valid())
        DEBUG_POINT;

    switch (m_eState)
    {
    case RESCUE_DIALOGUE_STATE::NONE:
        if (m_tComponents.animator.Is_Valid())
            m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::IDLE_CASUAL_M);
        break;

    case RESCUE_DIALOGUE_STATE::MOVE_TO_PLAYER:
        if (m_tComponents.animator.Is_Valid())
            m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RUN_CASUAL);
        break;

    case RESCUE_DIALOGUE_STATE::DIALOGUE:
        if (m_tComponents.animator.Is_Valid())
            m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::IDLE_CASUAL_M);

        Start_Dialogue();
        break;

    case RESCUE_DIALOGUE_STATE::EXIT:
        if (m_tComponents.animator.Is_Valid())
            m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::IDLE_CASUAL_M);

        Begin_StagingCameraExit();
        break;
    }
}

void CScoutBehavior_RescueDialogue::Process_None(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);

    if (m_scCinematicCamera.Is_Valid())
        DEBUG_POINT;
}

void CScoutBehavior_RescueDialogue::Process_MoveToPlayer(_float fDT)
{
    if (m_scCinematicCamera.Is_Valid())
        DEBUG_POINT;

    Update_StagingCamera(fDT);

    if (!m_goTargetPlayer)
        return;

    CTransform trPlayer = m_goTargetPlayer->Get_Component<CTransform>();
    if (!trPlayer.Is_Valid())
        return;

    _vector vCurPos = XMLoadFloat3(&m_tComponents.transform->vPosition);
    _vector vTargetPos = Get_PlayerApproachTargetXM();

    _vector vDiff = vTargetPos - vCurPos;
    _vector vDiffXZ = XMVectorSetY(vDiff, 0.f);

    _float fDist = XMVectorGetX(XMVector3Length(vDiffXZ));

    if (fDist <= m_fArriveDistance)
    {
        Look_At_WithModelReverse(XMLoadFloat3(&trPlayer->vPosition));
        Change_State(RESCUE_DIALOGUE_STATE::DIALOGUE);
        return;
    }

    if (XMVector3NearEqual(vDiffXZ, XMVectorZero(), XMVectorReplicate(0.0001f)))
        return;

    _vector vDir = XMVector3Normalize(vDiffXZ);

    _float fSpeed = m_fMoveSpeed;
    if (m_pStats)
        fSpeed = m_pStats->fCurSpeed;

    m_tComponents.transform.Translate(vDir * fSpeed * fDT, SPACE::WORLD);

    Look_At_WithModelReverse(XMLoadFloat3(&trPlayer->vPosition));
}

void CScoutBehavior_RescueDialogue::Process_Dialogue(_float fDT)
{
    Update_StagingCamera(fDT);
    Update_Dialogue(fDT);
}

void CScoutBehavior_RescueDialogue::Process_Exit(_float fDT)
{
    Update_StagingCamera(fDT);

    if (m_eStagingCameraState == STAGING_CAMERA_STATE::NONE)
    {
        if (m_goScout)
            m_goScout->Set_Enable(false);

        Set_FadeVisible(false);

        m_eState = RESCUE_DIALOGUE_STATE::NONE;
        m_eDialogueState = DIALOGUE_STATE::NONE;
    }
}

void CScoutBehavior_RescueDialogue::On_CinematicEvent(const CINEMATIC_EVENT_DATA& tEventData)
{
    if (tEventData.strEventName == "MOVETO")
    {
        Change_State(RESCUE_DIALOGUE_STATE::MOVE_TO_PLAYER);
        return;
    }
    else if (tEventData.strEventName == "FORCEEXIT")
    {
        SYS_CINEMATIC.Reset_Cinematic();
        Begin_StagingCamera_FromCurrentRender();
    }
}

void CScoutBehavior_RescueDialogue::Begin_StagingCamera_FromCurrentRender()
{
    if (!Is_StagingCameraReady())
        return;

    m_vStagingCameraStartPos = GAME_INSTANCE.Cam_Position();

    _float3 vCamLook3 = GAME_INSTANCE.Cam_Look();
    XMStoreFloat3(
        &m_vStagingCameraStartLookTarget,
        XMLoadFloat3(&m_vStagingCameraStartPos) + XMLoadFloat3(&vCamLook3) * 10.f);

    Build_StagingCameraTarget(m_vStagingCameraTargetPos, m_vStagingCameraTargetLookTarget);

    m_fStagingCameraLerpTime = 0.f;
    m_eStagingCameraState = STAGING_CAMERA_STATE::ENTER;

    Apply_StagingCameraPose(m_vStagingCameraStartPos, m_vStagingCameraStartLookTarget);

    m_goStagingCamera->Set_Enable(true);
    m_scStagingCamera.Set_Priority(255);
}

void CScoutBehavior_RescueDialogue::Begin_StagingCameraExit()
{
    if (!Is_StagingCameraReady())
        return;

    if (m_eStagingCameraState == STAGING_CAMERA_STATE::NONE)
        return;

    Build_StagingCameraTarget(m_vStagingCameraTargetPos, m_vStagingCameraTargetLookTarget);
    Apply_StagingCameraPose(m_vStagingCameraTargetPos, m_vStagingCameraTargetLookTarget);

    m_fStagingCameraLerpTime = 0.f;
    m_eStagingCameraState = STAGING_CAMERA_STATE::EXIT;
}

void CScoutBehavior_RescueDialogue::End_StagingCamera()
{
    if (!Is_StagingCameraReady())
        return;

    m_scStagingCamera.Set_Priority(0);
    m_goStagingCamera->Set_Enable(false);

    m_eStagingCameraState = STAGING_CAMERA_STATE::NONE;
}

void CScoutBehavior_RescueDialogue::Update_StagingCamera(_float fDT)
{
    if (!Is_StagingCameraReady())
        return;

    if (m_eStagingCameraState == STAGING_CAMERA_STATE::NONE)
        return;

    if (m_eStagingCameraState == STAGING_CAMERA_STATE::HOLD)
    {
        Build_StagingCameraTarget(m_vStagingCameraTargetPos, m_vStagingCameraTargetLookTarget);
        Apply_StagingCameraPose(m_vStagingCameraTargetPos, m_vStagingCameraTargetLookTarget);
        return;
    }

    m_fStagingCameraLerpTime += fDT;

    _float t = m_fStagingCameraLerpTime / m_fStagingCameraLerpDuration;
    t = CEasingFunction::Clamp01(t);
    t = CEasingFunction::EaseOutCubic(t);

    _float3 vCamPos{};
    _float3 vLookTarget{};

    if (m_eStagingCameraState == STAGING_CAMERA_STATE::ENTER)
    {
        Build_StagingCameraTarget(m_vStagingCameraTargetPos, m_vStagingCameraTargetLookTarget);

        vCamPos = CEasingFunction::Lerp(m_vStagingCameraStartPos, m_vStagingCameraTargetPos, t);
        vLookTarget = CEasingFunction::Lerp(m_vStagingCameraStartLookTarget, m_vStagingCameraTargetLookTarget, t);

        Apply_StagingCameraPose(vCamPos, vLookTarget);

        if (m_fStagingCameraLerpTime >= m_fStagingCameraLerpDuration)
        {
            Apply_StagingCameraPose(m_vStagingCameraTargetPos, m_vStagingCameraTargetLookTarget);
            m_eStagingCameraState = STAGING_CAMERA_STATE::HOLD;
        }

        return;
    }

    if (m_eStagingCameraState == STAGING_CAMERA_STATE::EXIT)
    {
        vCamPos = CEasingFunction::Lerp(m_vStagingCameraTargetPos, m_vStagingCameraStartPos, t);
        vLookTarget = CEasingFunction::Lerp(m_vStagingCameraTargetLookTarget, m_vStagingCameraStartLookTarget, t);

        Apply_StagingCameraPose(vCamPos, vLookTarget);

        if (m_fStagingCameraLerpTime >= m_fStagingCameraLerpDuration)
        {
            Apply_StagingCameraPose(m_vStagingCameraStartPos, m_vStagingCameraStartLookTarget);
            End_StagingCamera();
        }
    }
}

void CScoutBehavior_RescueDialogue::Build_StagingCameraTarget(_float3& vOutCamPos, _float3& vOutLookTarget) const
{
    vOutCamPos = m_tComponents.transform->vPosition;
    vOutLookTarget = m_tComponents.transform->vPosition;

    _vector vScoutPos = XMLoadFloat3(&m_tComponents.transform->vPosition);
    _vector vScoutRight = XMVector3Normalize(m_tComponents.transform.Get_StateXM(STATE::RIGHT));
    _vector vScoutUp = XMVector3Normalize(m_tComponents.transform.Get_StateXM(STATE::UP));
    _vector vScoutLook = XMVector3Normalize(m_tComponents.transform.Get_StateXM(STATE::LOOK));

    _vector vCamPos =
        vScoutPos +
        vScoutRight * 0.f +
        vScoutUp * 2.8f -
        vScoutLook * 4.5f;

    _vector vLookTarget =
        vScoutPos +
        vScoutUp * 1.7f;

    XMStoreFloat3(&vOutCamPos, vCamPos);
    XMStoreFloat3(&vOutLookTarget, vLookTarget);
}

void CScoutBehavior_RescueDialogue::Apply_StagingCameraPose(const _float3& vCamPos, const _float3& vLookTarget)
{
    if (!Is_StagingCameraReady())
        return;

    m_trStagingCamera->vPosition = vCamPos;
    m_trStagingCamera.Look_At(XMLoadFloat3(&vLookTarget));
}

_bool CScoutBehavior_RescueDialogue::Is_StagingCameraReady() const
{
    return m_goStagingCamera != nullptr
        && m_scStagingCamera.Is_Valid()
        && m_trStagingCamera.Is_Valid();
}

void CScoutBehavior_RescueDialogue::Start_Dialogue()
{
    m_eDialogueState = DIALOGUE_STATE::TYPING;

    m_iCurDialogueLine = 0;
    m_iCurDialogueChar = 0;

    m_fDialogueCharTime = 0.f;
    m_fDialogueWaitTime = 0.f;

    m_fFadeAlpha = 0.f;
    m_fFadeTime = 0.f;

    Set_DialogueVisible(true);
    Set_FadeVisible(false);
    Set_DialogueText(L"");
}

void CScoutBehavior_RescueDialogue::Update_Dialogue(_float fDT)
{
    if (m_vecDialogueLines.empty())
        return;

    switch (m_eDialogueState)
    {
    case DIALOGUE_STATE::TYPING:
    {
        const std::wstring& strLine = m_vecDialogueLines[m_iCurDialogueLine];

        m_fDialogueCharTime += fDT;

        while (m_fDialogueCharTime >= m_fDialogueCharInterval)
        {
            m_fDialogueCharTime -= m_fDialogueCharInterval;

            if (m_iCurDialogueChar < strLine.size())
                ++m_iCurDialogueChar;
        }

        Set_DialogueText(strLine.substr(0, m_iCurDialogueChar));

        if (m_iCurDialogueChar >= strLine.size())
        {
            m_fDialogueWaitTime = 0.f;
            m_eDialogueState = DIALOGUE_STATE::WAIT;
        }

        return;
    }

    case DIALOGUE_STATE::WAIT:
        m_fDialogueWaitTime += fDT;

        if (m_fDialogueWaitTime < m_fDialogueLineWaitDuration)
            return;

        if (m_iCurDialogueLine + 1 < m_vecDialogueLines.size())
        {
            ++m_iCurDialogueLine;
            m_iCurDialogueChar = 0;
            m_fDialogueCharTime = 0.f;
            m_fDialogueWaitTime = 0.f;

            Set_DialogueText(L"");
            m_eDialogueState = DIALOGUE_STATE::TYPING;
            return;
        }

        if (m_fDialogueWaitTime >= m_fDialogueEndWaitDuration)
        {
            m_eDialogueState = DIALOGUE_STATE::FADE_OUT;
            m_fFadeTime = 0.f;
            m_fFadeAlpha = 0.f;
            Set_FadeVisible(true);
        }

        return;

    case DIALOGUE_STATE::FADE_OUT:
        Update_Fade(fDT);
        return;

    case DIALOGUE_STATE::DONE:
        return;
    }
}

void CScoutBehavior_RescueDialogue::Update_Fade(_float fDT)
{
    m_fFadeTime += fDT;

    _float t = m_fFadeTime / m_fFadeDuration;
    t = CEasingFunction::Clamp01(t);
    t = CEasingFunction::EaseOutCubic(t);

    m_fFadeAlpha = t;

    if (m_crFadeUI.Is_Valid())
    {
        _float4 vColor = { 0.f, 0.f, 0.f, m_fFadeAlpha };
        m_crFadeUI.Set_Color(vColor);
    }

    if (t >= 0.7f)
    {
        if (m_crDialoguePanel.Is_Valid())
            m_crDialoguePanel.Set_Enable(false);

        if (m_txtDialogue.Is_Valid())
            m_txtDialogue.Set_Enable(false);

        if (m_tComponents.meshRenderer.Is_Valid())
            m_tComponents.meshRenderer.Set_Enable(false);
    }

    if (m_fFadeTime >= m_fFadeDuration)
    {
        m_eDialogueState = DIALOGUE_STATE::DONE;

        Set_DialogueVisible(false);

        if (m_tComponents.meshRenderer.Is_Valid())
            m_tComponents.meshRenderer.Set_Enable(false);

        /* 여기서 페이드 끄면 안 됨 */
        Change_State(RESCUE_DIALOGUE_STATE::EXIT);
    }
}

void CScoutBehavior_RescueDialogue::Set_DialogueText(const std::wstring& strText)
{
    if (!m_txtDialogue.Is_Valid())
        return;

    m_txtDialogue.Set_Text(strText);
}

void CScoutBehavior_RescueDialogue::Set_DialogueVisible(_bool bVisible)
{
    if (m_goDialogueUI)
        m_goDialogueUI->Set_Enable(bVisible);

    if (!bVisible && m_txtDialogue.Is_Valid())
        m_txtDialogue.Set_Text(L"");

    if (bVisible)
    {
        if (m_crDialoguePanel.Is_Valid())
            m_crDialoguePanel.Set_Enable(true);

        if (m_txtDialogue.Is_Valid())
            m_txtDialogue.Set_Enable(true);
    }
}

void CScoutBehavior_RescueDialogue::Set_FadeVisible(_bool bVisible)
{
    if (m_goFadeUI)
        m_goFadeUI->Set_Enable(bVisible);

    if (m_crFadeUI.Is_Valid())
    {
        _float4 vColor = { 0.f, 0.f, 0.f, bVisible ? m_fFadeAlpha : 0.f };
        m_crFadeUI.Set_Color(vColor);
    }
}

_vector CScoutBehavior_RescueDialogue::Get_PlayerApproachTargetXM() const
{
    if (!m_goTargetPlayer)
        return XMLoadFloat3(&m_tComponents.transform->vPosition);

    CTransform trPlayer = m_goTargetPlayer->Get_Component<CTransform>();
    if (!trPlayer.Is_Valid())
        return XMLoadFloat3(&m_tComponents.transform->vPosition);

    _vector vPlayerPos = XMLoadFloat3(&trPlayer->vPosition);

    return vPlayerPos - XMVectorSet(0.f, 0.f, m_fKeepDistanceFromPlayer, 0.f);
}

void CScoutBehavior_RescueDialogue::Look_At_WithModelReverse(_vector vTargetPos)
{
    _vector vCurPos = XMLoadFloat3(&m_tComponents.transform->vPosition);

    _vector vDir = vTargetPos - vCurPos;
    vDir = XMVectorSetY(vDir, 0.f);

    if (XMVector3NearEqual(vDir, XMVectorZero(), XMVectorReplicate(0.0001f)))
        return;

    vDir = XMVector3Normalize(vDir);

    m_tComponents.transform.Look_At(vCurPos - vDir);
}

NS_END
