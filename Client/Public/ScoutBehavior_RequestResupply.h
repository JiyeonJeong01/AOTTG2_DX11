#pragma once
#include "ScoutBehavior.h"
#include "Cinematic_Event.h"

NS_BEGIN(Client)

class CPlayer;
class CScout;
class CUI_NoticeController;

class CScoutBehavior_RequestResupply final : public CScoutBehavior
{
public:
    enum class RESUPPLY_STATE : uint32_t
    {
        NONE,
        REQUEST_NOTICE,
        IDLE_WAIT,
        DETECTED,
        APPROACH,
        RESUPPLY,
        SPECIAL,
        DIRECTING,
        EXIT
    };

private:
    struct OUTLINE_DESC
    {
        CMeshRenderer mr{};
        _float fOutlineWidth = 0.f;

        OUTLINE_DESC() = default;
        OUTLINE_DESC(CMeshRenderer _mr, _float _fOutlineWidth)
            : mr(_mr), fOutlineWidth(_fOutlineWidth)
        {
        }
    };

    enum class DIALOGUE_STATE : uint32_t
    {
        NONE,
        LINE_0_TYPING,
        LINE_0_WAIT,
        LINE_1_TYPING,
        LINE_1_WAIT,
        FADE_OUT,
        DONE
    };

    enum class SPECIAL_CAMERA_STATE : uint32_t
    {
        NONE,
        ENTER,
        HOLD,
        EXIT
    };

public:
    CScoutBehavior_RequestResupply(
        Engine::CGameObject* goScout,
        CScout* scScout,
        SCOUT_BEHAVIOR eBehavior);

    virtual ~CScoutBehavior_RequestResupply();

public:
    void Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

public:
    void Process_Start();

    void Set_SpecialCamera(CGameObject* pSpecial, CGameObject* pCinematic);
    void Set_UI(CUI_NoticeController* pNotice, CGameObject* pDialogue, CGameObject* pFade);
private:
    HRESULT SetUp_References();


    void Process_IdleWait(_float fDT);
    void Process_Detected(_float fDT);
    void Process_RequestNotice(_float fDT);
    void Process_Approach(_float fDT);
    void Process_Resupply();
    void Process_Special(_float fDT);
    void Process_Directing(_float fDT);
    void Process_Exit(_float fDT);

    void Play_WaitingGesture();
    void Play_DetectedGesture();

    void Set_Outline(_bool bEnable);
    void On_DetectedPlayer(_bool bDetected, CGameObject* pPlayer);
    _bool Is_Player(const CGameObject* pOther) const;
    void Change_State(RESUPPLY_STATE eState);

    void Start_Directing();
    void Finish_Directing();

    void OnTriggerEnter(const COLLISION_DESC& tCollisionDesc);
    void OnTriggerExit(const COLLISION_DESC& tCollisionDesc);
    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_CinematicEvent(const CINEMATIC_EVENT_DATA& tEventData);

    void Place_RequestNoticeCamera();

private:
    void Begin_SpecialCamera();
    void End_SpecialCamera();
    void Update_SpecialCamera(_float fDT);
    void Build_SpecialCameraTarget(_float3& vOutCamPos, _float3& vOutLookTarget) const;
    void Apply_SpecialCameraPose(const _float3& vCamPos, const _float3& vLookTarget);
    _bool Is_SpecialCameraReady() const;

private:
    void Start_Dialogue();
    void Update_Dialogue(_float fDT);
    void Advance_DialogueLine();
    void Set_DialogueText(const std::wstring& strText);
    void Set_DialogueVisible(_bool bVisible);
    void Set_FadeVisible(_bool bVisible);
    void Update_Fade(_float fDT);


private:
    CUI_NoticeController*           m_pNotice = nullptr;
    RESUPPLY_STATE                   m_eState = RESUPPLY_STATE::NONE;

    _bool                            m_bPlayerDetected = false;
    CGameObject*                     m_goDetectedPlayer = nullptr;
    _bool                            m_bAnimFinished = false;
    _bool                            m_bStartedDirecting = false;
    _bool                            m_bSpecialAnimPlayed = false;

    _float                           m_fIdleAnimWaitTime = 0.f;
    _float                           m_fDetectedGestureTime = 0.f;
    _bool                            m_bWaitingIdleLoop = false;

    std::vector<OUTLINE_DESC>        m_vecOutlines{};

    _float                           m_fStopAnimationDist = 8.f;
    _float                           m_fBehindDistance = 2.f;
    _float                           m_fResupplyDistance = 0.5f;
    _float3                          m_vExitPos = { 0.f, 0.f, 0.f };

    _bool                           m_bNoticeShown = false;
    _float                          m_fRequestNoticeTime = 0.f;
    _float                          m_fRequestNoticeDuration = 4.f;


private :
    CGameObject*                    m_goCinematicCamera = nullptr;
    CCamera                         m_scCinematicCamera{};


private:
    CGameObject*                     m_goSpecialCamera = nullptr;
    CCamera                          m_scSpecialCamera{};
    CTransform                       m_trSpecialCamera{};

    SPECIAL_CAMERA_STATE             m_eSpecialCameraState = SPECIAL_CAMERA_STATE::NONE;

    _float3                          m_vSpecialCameraStartPos = { 0.f, 0.f, 0.f };
    _float3                          m_vSpecialCameraStartLookTarget = { 0.f, 0.f, 0.f };
    _float3                          m_vSpecialCameraTargetPos = { 0.f, 0.f, 0.f };
    _float3                          m_vSpecialCameraTargetLookTarget = { 0.f, 0.f, 0.f };

    _float                           m_fSpecialCameraLerpTime = 0.f;
    _float                           m_fSpecialCameraLerpDuration = 0.75f;

    _float                           m_fDirectingTime = 0.f;
    _float                           m_fDirectingDuration = 1.5f;

private:

    CGameObject*                    m_goFadeUI = nullptr;
    CCanvasRenderer                 m_crFadeUI{};

    CGameObject*                    m_goDialogueUI = nullptr;
    CCanvasRenderer                 m_crDialoguePanel{};
    CUIText                         m_txtDialogue{};

    DIALOGUE_STATE                   m_eDialogueState = DIALOGUE_STATE::NONE;

    std::vector<std::wstring>        m_vecDialogueLines{};

    _uint                            m_iCurDialogueLine = 0;
    _uint                            m_iCurDialogueChar = 0;

    _float                           m_fDialogueCharTime = 0.f;
    _float                           m_fDialogueCharInterval = 0.1f;      /* 대사 글자 출력 */

    _float                           m_fDialogueWaitTime = 0.f;
    _float                           m_fDialogueLineWaitDuration = 0.5f;    /* 대사 종료 후 다음 라인 */
    _float                           m_fDialogueEndWaitDuration = 0.9f;     /* 대화 종료 후 페이드 대기 */

    _float                           m_fFadeAlpha = 0.f;
    _float                           m_fFadeDuration = 5.f;                /* 페이드 속도 */
    _float                           m_fFadeTime = 0.f;

};

NS_END
