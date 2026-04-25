#pragma once
#include "ScoutBehavior.h"
#include "Cinematic_Event.h"

NS_BEGIN(Client)

class CScout;
class CUI_NoticeController;

class CScoutBehavior_RescueDialogue final : public CScoutBehavior
{
public:
    enum class RESCUE_DIALOGUE_STATE : uint32_t
    {
        NONE,
        MOVE_TO_PLAYER,
        DIALOGUE,
        EXIT
    };

private:
    enum class DIALOGUE_STATE : uint32_t
    {
        NONE,
        TYPING,
        WAIT,
        FADE_OUT,
        DONE
    };

    enum class STAGING_CAMERA_STATE : uint32_t
    {
        NONE,
        ENTER,
        HOLD,
        EXIT
    };

public:
    CScoutBehavior_RescueDialogue(
        Engine::CGameObject* goScout,
        CScout* scScout,
        SCOUT_BEHAVIOR eBehavior);

    virtual ~CScoutBehavior_RescueDialogue();

public:
    void Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

public:
    void Process_Start(CGameObject* pPlayer);

    void Set_Cameras(CGameObject* pStagingCamera, CGameObject* pCinematicCamera);
    void Set_UI(CUI_NoticeController* pNotice, CGameObject* pDialogue, CGameObject* pFade);

private:
    void Change_State(RESCUE_DIALOGUE_STATE eState);

    void Process_None(_float fDT);
    void Process_MoveToPlayer(_float fDT);
    void Process_Dialogue(_float fDT);
    void Process_Exit(_float fDT);

private:
    void On_CinematicEvent(const CINEMATIC_EVENT_DATA& tEventData);

private:
    void Begin_StagingCamera_FromCurrentRender();
    void Begin_StagingCameraExit();
    void End_StagingCamera();
    void Update_StagingCamera(_float fDT);

    void Build_StagingCameraTarget(_float3& vOutCamPos, _float3& vOutLookTarget) const;
    void Apply_StagingCameraPose(const _float3& vCamPos, const _float3& vLookTarget);
    _bool Is_StagingCameraReady() const;

private:
    void Start_Dialogue();
    void Update_Dialogue(_float fDT);
    void Update_Fade(_float fDT);

    void Set_DialogueText(const std::wstring& strText);
    void Set_DialogueVisible(_bool bVisible);
    void Set_FadeVisible(_bool bVisible);

private:
    _vector Get_PlayerApproachTargetXM() const;
    void Look_At_WithModelReverse(_vector vTargetPos);

private:
    CUI_NoticeController* m_pNotice = nullptr;

    RESCUE_DIALOGUE_STATE            m_eState = RESCUE_DIALOGUE_STATE::NONE;

    CGameObject*                    m_goTargetPlayer = nullptr;

    _float                           m_fMoveSpeed = 6.f;
    _float                           m_fKeepDistanceFromPlayer = 2.2f;
    _float                           m_fArriveDistance = 0.45f;

private:
    CGameObject* m_goCinematicCamera = nullptr;
    CCamera                          m_scCinematicCamera{};

private:
    CGameObject* m_goStagingCamera = nullptr;
    CCamera                          m_scStagingCamera{};
    CTransform                       m_trStagingCamera{};

    STAGING_CAMERA_STATE             m_eStagingCameraState = STAGING_CAMERA_STATE::NONE;

    _float3                          m_vStagingCameraStartPos = { 0.f, 0.f, 0.f };
    _float3                          m_vStagingCameraStartLookTarget = { 0.f, 0.f, 0.f };
    _float3                          m_vStagingCameraTargetPos = { 0.f, 0.f, 0.f };
    _float3                          m_vStagingCameraTargetLookTarget = { 0.f, 0.f, 0.f };

    _float                           m_fStagingCameraLerpTime = 0.f;
    _float                           m_fStagingCameraLerpDuration = 1.2f;

private:
    CGameObject* m_goFadeUI = nullptr;
    CCanvasRenderer                  m_crFadeUI{};

    CGameObject* m_goDialogueUI = nullptr;
    CCanvasRenderer                  m_crDialoguePanel{};
    CUIText                          m_txtDialogue{};

    DIALOGUE_STATE                   m_eDialogueState = DIALOGUE_STATE::NONE;

    std::vector<std::wstring>        m_vecDialogueLines{};

    _uint                            m_iCurDialogueLine = 0;
    _uint                            m_iCurDialogueChar = 0;

    _float                           m_fDialogueCharTime = 0.f;
    _float                           m_fDialogueCharInterval = 0.08f;

    _float                           m_fDialogueWaitTime = 0.f;
    _float                           m_fDialogueLineWaitDuration = 0.55f;
    _float                           m_fDialogueEndWaitDuration = 0.9f;

    _float                           m_fFadeAlpha = 0.f;
    _float                           m_fFadeDuration = 2.2f;
    _float                           m_fFadeTime = 0.f;
};

NS_END
