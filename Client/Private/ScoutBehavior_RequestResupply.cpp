#include "ScoutBehavior_RequestResupply.h"
#include "Player.h"
#include "AnimationClip_Player.h"

#include "Easing_Function.h"
#include "CinematicSystem.h"
#include "UI_NoticeController.h"
#include "VFX_Manager.h"

NS_BEGIN(Client)
    CScoutBehavior_RequestResupply::CScoutBehavior_RequestResupply(
    Engine::CGameObject* goScout,
    CScout* scScout,
    SCOUT_BEHAVIOR eBehavior)
    : CScoutBehavior(goScout, scScout, eBehavior)
{
}

CScoutBehavior_RequestResupply::~CScoutBehavior_RequestResupply()
{
    if (m_scSpecialCamera.Is_Valid())
    {
        m_scSpecialCamera.Set_Priority(0);
    }

    if (m_goSpecialCamera)
        m_goSpecialCamera->Set_Enable(false);
}

void CScoutBehavior_RequestResupply::Initialize()
{
    SetUp_References();

    /* 값 초기화 */
    {
        m_bPlayerDetected = false;
        m_goDetectedPlayer = nullptr;
        m_bAnimFinished = false;
        m_bStartedDirecting = false;
        m_bSpecialAnimPlayed = false;

        m_fIdleAnimWaitTime = 0.f;
        m_fDetectedGestureTime = 0.f;
        m_bWaitingIdleLoop = false;

        m_eSpecialCameraState = SPECIAL_CAMERA_STATE::NONE;
        m_fSpecialCameraLerpTime = 0.f;
        m_fDirectingTime = 0.f;

        m_eDialogueState = DIALOGUE_STATE::NONE;
        m_iCurDialogueLine = 0;
        m_iCurDialogueChar = 0;
        m_fDialogueCharTime = 0.f;
        m_fDialogueWaitTime = 0.f;
        m_fFadeAlpha = 0.f;
        m_fFadeTime = 0.f;
    }

    /* 대사 등록 */
    {
        m_vecDialogueLines.clear();
        m_vecDialogueLines.emplace_back(L"구해줘서 고마워!!");
        m_vecDialogueLines.emplace_back(L"탈환 후 살아서 보자!");
    }

    /* 아웃라인 등록 */
    {
        m_vecOutlines.clear();

        /* 본체 MeshRenderer */
        {
            CMeshRenderer mr = m_goScout->Get_Component<CMeshRenderer>();
            if (mr.Is_Valid())
            {
                if (mr->hPerObjectParams == INVALID_HANDLE_UINT)
                    mr->hPerObjectParams = GAME_INSTANCE.Alloc_PerObjectParamBlock();

                _float fWidth = 5.f;

                PER_OBJECT_PARAM_BLOCK* pBlock = GAME_INSTANCE.Get_PerObjectParamBlock(mr->hPerObjectParams);
                if (pBlock)
                {
                    pBlock->block.Set_Float("g_OutlineWidth", fWidth);
                    pBlock->block.Set_Float4("g_OutlineColor", { 1.f, 1.f, 1.f, 1.f });
                }

                m_vecOutlines.emplace_back(mr, fWidth);
            }
        }

        /* 자식 MeshRenderer 전부 수집 */
        {
            auto vecChildren = m_goScout->Get_Children();

            for (CGameObject* pChild : vecChildren)
            {
                if (!pChild)
                    continue;

                CMeshRenderer mr = pChild->Get_Component<CMeshRenderer>();
                if (!mr.Is_Valid())
                    continue;

                if (mr->hPerObjectParams == INVALID_HANDLE_UINT)
                    mr->hPerObjectParams = GAME_INSTANCE.Alloc_PerObjectParamBlock();

                PER_OBJECT_PARAM_BLOCK* pBlock = GAME_INSTANCE.Get_PerObjectParamBlock(mr->hPerObjectParams);

                _float fWidth = 0.015f;

                if (pChild->Get_Label() == "Head")
                    fWidth = 0.001f;

                if (pBlock)
                {
                    pBlock->block.Set_Float("g_OutlineWidth", fWidth);
                    pBlock->block.Set_Float4("g_OutlineColor", { 1.f, 1.f, 1.f, 1.f });
                }

                m_vecOutlines.emplace_back(mr, fWidth);
            }
        }

        IF_TRUE_RETURN_MSG_BREAK(m_vecOutlines.empty(), , "all scout mesh renderers are invalid");

        Set_Outline(false);
    }

    m_tComponents.animator->OnAnimationFinished.Add_Listener(
        &CScoutBehavior_RequestResupply::On_AnimFinished, this);

    m_eState = RESUPPLY_STATE::NONE;
}

void CScoutBehavior_RequestResupply::Process_Start()
{
    Change_State(RESUPPLY_STATE::NONE);

    SYS_CINEMATIC.Subscribe_CinematicEvent(
        "Scout_RequestResupply",
        CINEMATIC_EVENT_TYPE::CUSTOM,
        &CScoutBehavior_RequestResupply::On_CinematicEvent,
        this);

    SYS_CINEMATIC.Play("Scout_RequestResupply", m_scCinematicCamera);
}

void CScoutBehavior_RequestResupply::Set_Managers(CVFX_Manager* pVFXMgr)
{
    m_pVFXMgr = pVFXMgr;
}

void CScoutBehavior_RequestResupply::Priority_Update(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);
}

void CScoutBehavior_RequestResupply::Update(_float fDT)
{
    switch (m_eState)
    {
    case RESUPPLY_STATE::NONE:
        Process_None(fDT);
        return;

    case RESUPPLY_STATE::IDLE_WAIT:
        Process_IdleWait(fDT);
        break;

    case RESUPPLY_STATE::DETECTED:
        Process_Detected(fDT);
        break;
    case RESUPPLY_STATE::REQUEST_NOTICE:
        Process_RequestNotice(fDT);
        break;

    case RESUPPLY_STATE::APPROACH:
        Process_Approach(fDT);
        break;

    case RESUPPLY_STATE::RESUPPLY:
        Process_Resupply();
        break;

    case RESUPPLY_STATE::SPECIAL:
        Process_Special(fDT);
        break;

    case RESUPPLY_STATE::DIRECTING:
        Process_Directing(fDT);
        break;

    case RESUPPLY_STATE::EXIT:
        Process_Exit(fDT);
        break;
    }
}

void CScoutBehavior_RequestResupply::Late_Update(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);
}

HRESULT CScoutBehavior_RequestResupply::SetUp_References()
{
    CCollider trigger = m_goScout->Get_Component<CCollider>();
    IF_TRUE_RETURN_MSG_BREAK(!trigger.Is_Valid(), E_FAIL, "trigger is invalid");

    trigger->OnTriggerEnter.Add_Listener(&CScoutBehavior_RequestResupply::OnTriggerEnter, this);
    trigger->OnTriggerExit.Add_Listener(&CScoutBehavior_RequestResupply::OnTriggerExit, this);

    return S_OK;
}

void CScoutBehavior_RequestResupply::Process_None(_float fDT)
{
    if (m_bSignalFlarePlaying)
    {
        Handle_SignalFlare(fDT);
    }
}

void CScoutBehavior_RequestResupply::Change_State(RESUPPLY_STATE eState)
{
    m_eState = eState;

    switch (m_eState)
    {
    case RESUPPLY_STATE::IDLE_WAIT:
        m_fIdleAnimWaitTime = 0.f;
        m_bWaitingIdleLoop = false;
        Set_Outline(false);
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::IDLE_CASUAL_M);
        break;

    case RESUPPLY_STATE::REQUEST_NOTICE:
        m_fRequestNoticeTime = 0.f;
        m_bNoticeShown = false;

        Place_RequestNoticeCamera();
        break;

    case RESUPPLY_STATE::DETECTED:
        m_fDetectedGestureTime = 0.f;
        Set_Outline(true);
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::IDLE_CASUAL_M);
        Play_DetectedGesture();
        break;

    case RESUPPLY_STATE::APPROACH:
        Set_Outline(false);
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RUN);
        break;

    case RESUPPLY_STATE::RESUPPLY:
        m_bAnimFinished = false;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RESUPPLY);
        break;

    case RESUPPLY_STATE::SPECIAL:
        m_bAnimFinished = false;
        m_bSpecialAnimPlayed = false;
        Begin_SpecialCamera();
        break;

    case RESUPPLY_STATE::DIRECTING:
        m_bStartedDirecting = false;
        m_fDirectingTime = 0.f;
        break;

    case RESUPPLY_STATE::EXIT:
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RUN);
        break;
    }
}

void CScoutBehavior_RequestResupply::Process_IdleWait(_float fDT)
{
    if (m_goDetectedPlayer)
    {
        Change_State(RESUPPLY_STATE::DETECTED);
        return;
    }

    m_fIdleAnimWaitTime += fDT;

    if (m_fIdleAnimWaitTime >= 2.f)
    {
        Play_WaitingGesture();
        m_fIdleAnimWaitTime = 0.f;
    }
}

void CScoutBehavior_RequestResupply::Process_Detected(_float fDT)
{
    if (!m_goDetectedPlayer)
    {
        Change_State(RESUPPLY_STATE::IDLE_WAIT);
        return;
    }

    m_fDetectedGestureTime += fDT;

    if (m_fDetectedGestureTime >= 2.f)
    {
        Play_DetectedGesture();
        m_fDetectedGestureTime = 0.f;
    }

    CTransform trPlayer = m_goDetectedPlayer->Get_Component<CTransform>();
    if (!trPlayer.Is_Valid())
        return;

    _vector vPlayerPos = XMLoadFloat3(&trPlayer->vPosition);
    _vector vCurPos = XMLoadFloat3(&m_tComponents.transform->vPosition);

    _float fDist = XMVectorGetX(XMVector3Length(vPlayerPos - vCurPos));

    if (fDist <= m_fStopAnimationDist)
    {
        Change_State(RESUPPLY_STATE::APPROACH);
    }
}

void CScoutBehavior_RequestResupply::Process_RequestNotice(_float fDT)
{
    if (!m_bNoticeShown)
    {
        m_bNoticeShown = true;

        if (m_pNotice)
            m_pNotice->Show_Notice(NOTICE_TYPE::REQUEST_RESUPPLY, m_fRequestNoticeDuration);
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::EMOTE_NO);
    }

    m_fRequestNoticeTime += fDT;

    if (m_fRequestNoticeTime >= m_fRequestNoticeDuration)
    {
        End_SpecialCamera();
        Change_State(RESUPPLY_STATE::IDLE_WAIT);
    }
}

void CScoutBehavior_RequestResupply::Process_Approach(_float fDT)
{
    if (!m_goDetectedPlayer)
    {
        Change_State(RESUPPLY_STATE::IDLE_WAIT);
        return;
    }

    CTransform trPlayer = m_goDetectedPlayer->Get_Component<CTransform>();
    if (!trPlayer.Is_Valid())
        return;

    _vector vPlayerPos = XMLoadFloat3(&trPlayer->vPosition);
    _vector vLook = XMVector3Normalize(trPlayer.Get_StateXM(STATE::LOOK));

    /* 뒤로 이동해야 하므로 +가 아니라 - */
    _vector vTargetPos = vPlayerPos - vLook * m_fBehindDistance;

    _vector vCurPos = XMLoadFloat3(&m_tComponents.transform->vPosition);
    _vector vDiff = vTargetPos - vCurPos;

    _float fDist = XMVectorGetX(XMVector3Length(vDiff));
    if (fDist <= m_fResupplyDistance)
    {
        Change_State(RESUPPLY_STATE::RESUPPLY);
        return;
    }

    _vector vDir = XMVector3Normalize(XMVectorSetY(vDiff, 0.f));
    _vector vLookTarget = vCurPos - vDir;

    m_tComponents.transform.Translate(vDir * m_pStats->fCurSpeed * fDT, SPACE::WORLD);
    m_tComponents.transform.Look_At(vLookTarget);
}

void CScoutBehavior_RequestResupply::Process_Resupply()
{
    if (!m_goDetectedPlayer)
        return;
}

void CScoutBehavior_RequestResupply::Process_Special(_float fDT)
{
    if (!Is_SpecialCameraReady())
    {
        if (!m_bSpecialAnimPlayed)
        {
            m_bSpecialAnimPlayed = true;
            m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::SPECIAL_ARMIN);
        }
        return;
    }

    Update_SpecialCamera(fDT);

    if (m_eSpecialCameraState == SPECIAL_CAMERA_STATE::HOLD &&
        !m_bSpecialAnimPlayed)
    {
        m_bSpecialAnimPlayed = true;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::SPECIAL_ARMIN);
    }
}

void CScoutBehavior_RequestResupply::Process_Directing(_float fDT)
{
    if (!m_bStartedDirecting)
    {
        Start_Directing();
        m_bStartedDirecting = true;
    }

    Update_Dialogue(fDT);

    if (m_eSpecialCameraState == SPECIAL_CAMERA_STATE::EXIT)
        Update_SpecialCamera(fDT);
}

void CScoutBehavior_RequestResupply::Process_Exit(_float fDT)
{
    _vector vExit = XMLoadFloat3(&m_vExitPos);
    _vector vCurPos = XMLoadFloat3(&m_tComponents.transform->vPosition);

    _vector vDir = vExit - vCurPos;
    _vector vDirXZ = XMVectorSetY(vDir, 0.f);

    if (!XMVector3Equal(vDirXZ, XMVectorZero()))
        m_tComponents.transform.Look_At(XMVector3Normalize(vDirXZ));

    m_tComponents.transform.Translate(XMVector3Normalize(vDirXZ) * m_pStats->fMaxSpeed * fDT, SPACE::WORLD);

    _float fDist = XMVectorGetX(XMVector3Length(vExit - XMLoadFloat3(&m_tComponents.transform->vPosition)));
    if (fDist < 0.2f)
    {
        m_goScout->Set_Enable(false);
    }
}

void CScoutBehavior_RequestResupply::Play_WaitingGesture()
{
    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::EMOTE_NO);
}

void CScoutBehavior_RequestResupply::Play_DetectedGesture()
{
    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::EMOTE_WAVE);
}

void CScoutBehavior_RequestResupply::Set_Outline(_bool bEnable)
{
    if (m_vecOutlines.empty())
        return;

    for (auto& outline : m_vecOutlines)
    {
        if (!outline.mr.Is_Valid())
            continue;

        if (bEnable)
            outline.mr->extraPassFlags |= To<uint32_t>(EXTRA_RENDER_PASS::OUTLINE);
        else
            outline.mr->extraPassFlags &= ~To<uint32_t>(EXTRA_RENDER_PASS::OUTLINE);

        auto* pBlock = GAME_INSTANCE.Get_PerObjectParamBlock(outline.mr->hPerObjectParams);
        if (!pBlock)
            continue;

        pBlock->block.Set_Float("g_OutlineWidth", outline.fOutlineWidth);
        pBlock->block.Set_Float4("g_OutlineColor", { 1.f, 1.f, 1.f, 1.f });
    }
}

void CScoutBehavior_RequestResupply::Start_Directing()
{
    Start_Dialogue();
}

void CScoutBehavior_RequestResupply::Finish_Directing()
{
    if (Is_SpecialCameraReady())
    {
        m_fSpecialCameraLerpTime = 0.f;
        m_eSpecialCameraState = SPECIAL_CAMERA_STATE::EXIT;
        return;
    }
}

void CScoutBehavior_RequestResupply::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    const _uint iIndex = tData.iAnimationClip;

    const _uint iIdleIdx = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::IDLE_CASUAL_M);
    const _uint iNoIdx = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::EMOTE_NO);
    const _uint iWaveIdx = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::EMOTE_WAVE);
    const _uint iResupplyIdx = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::RESUPPLY);
    const _uint iSpecialIdx = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::SPECIAL_ARMIN);

    if (m_eState == RESUPPLY_STATE::REQUEST_NOTICE)
    {
        if (iIndex == iNoIdx)
            m_tComponents.animator.Set_NextAnimationClip(iIdleIdx);

        return;
    }

    if (m_eState == RESUPPLY_STATE::IDLE_WAIT)
    {
        if (iIndex == iNoIdx)
            m_tComponents.animator.Set_NextAnimationClip(iIdleIdx);

        return;
    }

    if (m_eState == RESUPPLY_STATE::DETECTED)
    {
        if (iIndex == iWaveIdx)
            m_tComponents.animator.Set_NextAnimationClip(iIdleIdx);

        return;
    }

    if (m_eState == RESUPPLY_STATE::RESUPPLY)
    {
        if (iIndex == iResupplyIdx)
        {
            if (m_goDetectedPlayer)
            {
                CPlayer* pPlayer = m_goDetectedPlayer->Get_Script<CPlayer>();
                if (pPlayer)
                    pPlayer->Complete_Deliver_Supplies();
            }

            Change_State(RESUPPLY_STATE::SPECIAL);
        }

        return;
    }

    if (m_eState == RESUPPLY_STATE::SPECIAL)
    {
        if (iIndex == iSpecialIdx)
        {
            m_bAnimFinished = true;
            Change_State(RESUPPLY_STATE::DIRECTING);
        }

        return;
    }
}

void CScoutBehavior_RequestResupply::Place_RequestNoticeCamera()
{
    if (!Is_SpecialCameraReady())
        return;

    Build_SpecialCameraTarget(m_vSpecialCameraTargetPos, m_vSpecialCameraTargetLookTarget);

    Apply_SpecialCameraPose(m_vSpecialCameraTargetPos, m_vSpecialCameraTargetLookTarget);

    m_goSpecialCamera->Set_Enable(true);
    m_scSpecialCamera.Set_Priority(255);

    m_eSpecialCameraState = SPECIAL_CAMERA_STATE::HOLD;
    m_fSpecialCameraLerpTime = 0.f;
}

void CScoutBehavior_RequestResupply::On_CinematicEvent(const CINEMATIC_EVENT_DATA& tEventData)
{
    if (tEventData.strEventName == "FORCEEXIT")
    {
        SYS_CINEMATIC.Reset_Cinematic();
        Change_State(RESUPPLY_STATE::REQUEST_NOTICE);
    }
    else if (tEventData.strEventName == "SIGNALFLARE")
    {
        m_bSignalFlarePlaying = true;
        m_fSignalFlareTime = 0.f;
        m_fSignalSmokeAcc = 0.f;

        _vector vStartPos = XMLoadFloat3(&m_tComponents.transform->vPosition)
            + XMVectorSet(0.f, 1.5f, 0.f, 0.f);

        XMStoreFloat3(&m_vSignalFlarePos, vStartPos);
    }
}

void CScoutBehavior_RequestResupply::Start_Dialogue()
{
    m_eDialogueState = DIALOGUE_STATE::LINE_0_TYPING;
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

void CScoutBehavior_RequestResupply::Update_Dialogue(_float fDT)
{
    if (m_vecDialogueLines.empty())
        return;

    switch (m_eDialogueState)
    {
    case DIALOGUE_STATE::LINE_0_TYPING:
    case DIALOGUE_STATE::LINE_1_TYPING:
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

            if (m_eDialogueState == DIALOGUE_STATE::LINE_0_TYPING)
                m_eDialogueState = DIALOGUE_STATE::LINE_0_WAIT;
            else
                m_eDialogueState = DIALOGUE_STATE::LINE_1_WAIT;
        }
        break;
    }

    case DIALOGUE_STATE::LINE_0_WAIT:
        m_fDialogueWaitTime += fDT;
        if (m_fDialogueWaitTime >= m_fDialogueLineWaitDuration)
        {
            m_iCurDialogueLine = 1;
            m_iCurDialogueChar = 0;
            m_fDialogueCharTime = 0.f;
            Set_DialogueText(L"");
            m_eDialogueState = DIALOGUE_STATE::LINE_1_TYPING;
        }
        break;

    case DIALOGUE_STATE::LINE_1_WAIT:
        m_fDialogueWaitTime += fDT;
        if (m_fDialogueWaitTime >= m_fDialogueEndWaitDuration)
        {
            m_eDialogueState = DIALOGUE_STATE::FADE_OUT;
            m_fFadeTime = 0.f;
            m_fFadeAlpha = 0.f;
            Set_FadeVisible(true);
        }
        break;

    case DIALOGUE_STATE::FADE_OUT:
        Update_Fade(fDT);
        break;

    case DIALOGUE_STATE::DONE:
        break;
    }
}

void CScoutBehavior_RequestResupply::Update_Fade(_float fDT)
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

    /* fade 70% 진행 시 대화창 먼저 숨김 */
    if (t >= 0.7f)
    {
        if (m_crDialoguePanel.Is_Valid())
            m_crDialoguePanel.Set_Enable(false);

        if (m_txtDialogue.Is_Valid())
            m_txtDialogue.Set_Enable(false);
    }

    if (m_fFadeTime >= m_fFadeDuration)
    {
        m_eDialogueState = DIALOGUE_STATE::DONE;

        Set_DialogueVisible(false);
        Set_FadeVisible(false);

        Finish_Directing();

        /* 강제로 meshrenderer 꺼두기 */
        for (auto& mr : m_vecOutlines)
        {
            mr.mr.Set_Enable(false);
        }
    }
}

void CScoutBehavior_RequestResupply::Advance_DialogueLine()
{
    m_iCurDialogueChar = 0;
    m_fDialogueCharTime = 0.f;
    m_fDialogueWaitTime = 0.f;

    if (m_iCurDialogueLine == 0)
    {
        m_iCurDialogueLine = 1;
        m_eDialogueState = DIALOGUE_STATE::LINE_1_TYPING;
        Set_DialogueText(L"");
        return;
    }

    m_eDialogueState = DIALOGUE_STATE::LINE_1_WAIT;
}

void CScoutBehavior_RequestResupply::Set_DialogueVisible(_bool bVisible)
{
    if (m_goDialogueUI)
        m_goDialogueUI->Set_Enable(bVisible);

    if (!bVisible && m_txtDialogue.Is_Valid())
        m_txtDialogue.Set_Text(L"");
}

void CScoutBehavior_RequestResupply::Set_FadeVisible(_bool bVisible)
{
    if (m_goFadeUI)
        m_goFadeUI->Set_Enable(bVisible);

    if (m_crFadeUI.Is_Valid())
    {
        _float4 vColor = { 1.f, 1.f, 1.f, bVisible ? m_fFadeAlpha : 0.f };
        m_crFadeUI.Set_Color(vColor);
    }
}

void CScoutBehavior_RequestResupply::Set_DialogueText(const std::wstring& strText)
{
    if (!m_txtDialogue.Is_Valid())
        return;

    m_txtDialogue.Set_Text(strText);
}


void CScoutBehavior_RequestResupply::OnTriggerEnter(const COLLISION_DESC& tCollisionDesc)
{
    if (m_eState != RESUPPLY_STATE::IDLE_WAIT &&
        m_eState != RESUPPLY_STATE::DETECTED)
        return;

    CGameObject* pOther = GAME_INSTANCE.Find_GameObject(tCollisionDesc.hObject);
    if (!pOther)
        return;

    if (!Is_Player(pOther))
        return;

    On_DetectedPlayer(true, pOther);
    Change_State(RESUPPLY_STATE::DETECTED);
}

void CScoutBehavior_RequestResupply::OnTriggerExit(const COLLISION_DESC& tCollisionDesc)
{
    CGameObject* pOther = GAME_INSTANCE.Find_GameObject(tCollisionDesc.hObject);
    if (!pOther)
        return;

    if (pOther != m_goDetectedPlayer)
        return;

    On_DetectedPlayer(false, nullptr);

    if (m_eState == RESUPPLY_STATE::DETECTED)
        Change_State(RESUPPLY_STATE::IDLE_WAIT);
}

_bool CScoutBehavior_RequestResupply::Is_Player(const CGameObject* pOther) const
{
    if (!pOther)
        return false;

    return pOther->Is_ExactMask(O_PLAYER);
}

void CScoutBehavior_RequestResupply::On_DetectedPlayer(_bool bDetected, CGameObject* pPlayer)
{
    m_bPlayerDetected = bDetected;
    m_goDetectedPlayer = pPlayer;

    Set_Outline(bDetected);
}

_bool CScoutBehavior_RequestResupply::Is_SpecialCameraReady() const
{
    return m_goSpecialCamera != nullptr
        && m_scSpecialCamera.Is_Valid()
        && m_trSpecialCamera.Is_Valid();
}

void CScoutBehavior_RequestResupply::Build_SpecialCameraTarget(_float3& vOutCamPos, _float3& vOutLookTarget) const
{
    vOutCamPos = m_tComponents.transform->vPosition;
    vOutLookTarget = m_tComponents.transform->vPosition;

    _vector vScoutPos = XMLoadFloat3(&m_tComponents.transform->vPosition);
    _vector vScoutRight = XMVector3Normalize(m_tComponents.transform.Get_StateXM(STATE::RIGHT));
    _vector vScoutUp = XMVector3Normalize(m_tComponents.transform.Get_StateXM(STATE::UP));
    _vector vScoutLook = XMVector3Normalize(m_tComponents.transform.Get_StateXM(STATE::LOOK));

    /* (0, 3, -5) */
    _vector vCamPos =
        vScoutPos +
        vScoutRight * 0.f +
        vScoutUp * 3.f -
        vScoutLook * 5.f;

    /* scout 상체 쪽을 보게 */
    _vector vLookTarget =
        vScoutPos +
        vScoutUp * 1.8f;

    XMStoreFloat3(&vOutCamPos, vCamPos);
    XMStoreFloat3(&vOutLookTarget, vLookTarget);
}

void CScoutBehavior_RequestResupply::Apply_SpecialCameraPose(const _float3& vCamPos, const _float3& vLookTarget)
{
    if (!Is_SpecialCameraReady())
        return;

    m_tComponents.transform;
    m_trSpecialCamera->vPosition = vCamPos;
    m_trSpecialCamera.Look_At(XMLoadFloat3(&vLookTarget));
}

void CScoutBehavior_RequestResupply::Begin_SpecialCamera()
{
    if (!Is_SpecialCameraReady())
        return;

    m_vSpecialCameraStartPos = GAME_INSTANCE.Cam_Position();
    _float3 vCamLook3 = GAME_INSTANCE.Cam_Look();
    XMStoreFloat3(&m_vSpecialCameraStartLookTarget, XMLoadFloat3(&m_vSpecialCameraStartPos) + XMLoadFloat3(&vCamLook3) * 10.f);

    Build_SpecialCameraTarget(m_vSpecialCameraTargetPos, m_vSpecialCameraTargetLookTarget);

    m_fSpecialCameraLerpTime = 0.f;
    m_eSpecialCameraState = SPECIAL_CAMERA_STATE::ENTER;
    m_bSpecialAnimPlayed = false;

    Apply_SpecialCameraPose(m_vSpecialCameraStartPos, m_vSpecialCameraStartLookTarget);

    m_goSpecialCamera->Set_Enable(true);
    m_scSpecialCamera.Set_Priority(255);
}

void CScoutBehavior_RequestResupply::End_SpecialCamera()
{
    if (!Is_SpecialCameraReady())
        return;

    m_scSpecialCamera.Set_Priority(0);
    m_goSpecialCamera->Set_Enable(false);
    m_eSpecialCameraState = SPECIAL_CAMERA_STATE::NONE;
}

void CScoutBehavior_RequestResupply::Update_SpecialCamera(_float fDT)
{
    if (!Is_SpecialCameraReady())
        return;

    if (m_eSpecialCameraState == SPECIAL_CAMERA_STATE::NONE ||
        m_eSpecialCameraState == SPECIAL_CAMERA_STATE::HOLD)
        return;

    m_fSpecialCameraLerpTime += fDT;

    _float t = m_fSpecialCameraLerpTime / m_fSpecialCameraLerpDuration;
    t = CEasingFunction::EaseOutCubic(t);

    _float3 vCamPos{};
    _float3 vLookTarget{};

    if (m_eSpecialCameraState == SPECIAL_CAMERA_STATE::ENTER)
    {
        vCamPos = CEasingFunction::Lerp(m_vSpecialCameraStartPos, m_vSpecialCameraTargetPos, t);
        vLookTarget = CEasingFunction::Lerp(m_vSpecialCameraStartLookTarget, m_vSpecialCameraTargetLookTarget, t);

        Apply_SpecialCameraPose(vCamPos, vLookTarget);

        if (m_fSpecialCameraLerpTime >= m_fSpecialCameraLerpDuration)
        {
            Apply_SpecialCameraPose(m_vSpecialCameraTargetPos, m_vSpecialCameraTargetLookTarget);
            m_eSpecialCameraState = SPECIAL_CAMERA_STATE::HOLD;
        }
    }
    else if (m_eSpecialCameraState == SPECIAL_CAMERA_STATE::EXIT)
    {
        vCamPos = CEasingFunction::Lerp(m_vSpecialCameraTargetPos, m_vSpecialCameraStartPos, t);
        vLookTarget = CEasingFunction::Lerp(m_vSpecialCameraTargetLookTarget, m_vSpecialCameraStartLookTarget, t);

        Apply_SpecialCameraPose(vCamPos, vLookTarget);

        if (m_fSpecialCameraLerpTime >= m_fSpecialCameraLerpDuration)
        {
            Apply_SpecialCameraPose(m_vSpecialCameraStartPos, m_vSpecialCameraStartLookTarget);
            End_SpecialCamera();

            if (m_goScout)
                m_goScout->Set_Enable(false);
        }
    }
}

void CScoutBehavior_RequestResupply::Handle_SignalFlare(_float fDT)
{
    if (!m_bSignalFlarePlaying)
        return;

    m_fSignalFlareTime += fDT;
    m_fSignalSmokeAcc += fDT;

    m_fSignalFlareVerticalSpeed -= 9.8f * 0.35f * fDT;

    _vector vPos = XMLoadFloat3(&m_vSignalFlarePos);
    _vector vMoveDir = XMLoadFloat3(&m_vSignalFlareMoveDir);

    vPos += vMoveDir * m_fSignalFlareHorizontalSpeed * fDT;
    vPos += XMVectorSet(0.f, m_fSignalFlareVerticalSpeed * fDT, 0.f, 0.f);

    _float3 vPos3{};
    XMStoreFloat3(&vPos3, vPos);
    m_vSignalFlarePos = vPos3;

    if (m_fSignalSmokeAcc >= 0.05f)
    {
        m_fSignalSmokeAcc = 0.f;

        auto* pSmoke = m_pVFXMgr->Start_Particle(PARTICLE_VFX::SIGNAL_FLARE_SMOKE, m_vSignalFlarePos);
        if (pSmoke != nullptr)
        {
            _float3 vForward3 = { 0.f, 0.f, 1.f };

            pSmoke->meshRenderer.Set_ParticleForward(vForward3);
            pSmoke->meshRenderer.Reset_Particle();
            pSmoke->meshRenderer.Set_ParticlePlaying(true);
        }
    }

    if (m_fSignalFlareTime >= m_fSignalFlareDuration)
    {
        m_bSignalFlarePlaying = false;
    }
}

void CScoutBehavior_RequestResupply::Set_SpecialCamera(CGameObject* pCameraObject, CGameObject* pCinematic)
{
    /* special */
    m_goSpecialCamera = pCameraObject;

    m_scSpecialCamera = m_goSpecialCamera->Get_Component<CCamera>();
    m_trSpecialCamera = m_goSpecialCamera->Get_Component<CTransform>();

    IF_TRUE_RETURN_MSG_BREAK(!m_scSpecialCamera.Is_Valid(), , "special camera component is invalid");
    IF_TRUE_RETURN_MSG_BREAK(!m_trSpecialCamera.Is_Valid(), , "special camera transform is invalid");

    m_goSpecialCamera->Set_Enable(false);
    m_scSpecialCamera.Set_Priority(0);

    /* cinematic */
    m_goCinematicCamera = pCinematic;
    m_scCinematicCamera = m_goCinematicCamera->Get_Component<CCamera>();

    IF_TRUE_RETURN_MSG_BREAK(!m_scCinematicCamera.Is_Valid(), , "special camera component is invalid");
}

void CScoutBehavior_RequestResupply::Set_UI(CUI_NoticeController* pNotice, CGameObject* pDialogue, CGameObject* pFade)
{
    m_pNotice = pNotice;

    m_goDialogueUI = pDialogue;
    m_crDialoguePanel = {};
    m_txtDialogue = {};

    if (!m_goDialogueUI)
        return;

    m_crDialoguePanel = m_goDialogueUI->Get_Component<CCanvasRenderer>();
    m_txtDialogue = m_goDialogueUI->Get_Component<CUIText>();

    IF_TRUE_RETURN_MSG_BREAK(!m_crDialoguePanel.Is_Valid(), , "dialogue panel canvas renderer is invalid");
    IF_TRUE_RETURN_MSG_BREAK(!m_txtDialogue.Is_Valid(), , "dialogue text is invalid");

    m_goDialogueUI->Set_Enable(false);

    m_goFadeUI = pFade;
    m_crFadeUI = {};

    if (!m_goFadeUI)
        return;

    m_crFadeUI = m_goFadeUI->Get_Component<CCanvasRenderer>();
    IF_TRUE_RETURN_MSG_BREAK(!m_crFadeUI.Is_Valid(), , "fade ui canvas renderer is invalid");

    m_goFadeUI->Set_Enable(false);

    _float4 vColor = { 1.f, 1.f, 1.f, 0.f };
    m_crFadeUI.Set_Color(vColor);
}

NS_END
