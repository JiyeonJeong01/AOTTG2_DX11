#include "ScoutBehavior_RequestResupply.h"
#include "Player.h"
#include "AnimationClip_Player.h"

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
}

void CScoutBehavior_RequestResupply::Initialize()
{
    SetUp_References();

    m_bPlayerDetected = false;
    m_goDetectedPlayer = nullptr;

    m_mr = m_goScout->Get_Component<CMeshRenderer>();
    IF_TRUE_RETURN_MSG_BREAK(!m_mr.Is_Valid(), , "mr is invalid");

    if (m_mr->hPerObjectParams == INVALID_HANDLE_UINT)
        m_mr->hPerObjectParams = GAME_INSTANCE.Alloc_PerObjectParamBlock();

    PER_OBJECT_PARAM_BLOCK* pBlock = GAME_INSTANCE.Get_PerObjectParamBlock(m_mr->hPerObjectParams);
    IF_NULL_RETURN_MSG_BREAK(pBlock, , "pBlock is nullptr");

    pBlock->block.Set_Float("g_OutlineWidth", m_fOutlineWidth);
    pBlock->block.Set_Float4("g_OutlineColor", { 1.f, 1.f, 1.f, 1.f });

    m_mr->extraPassFlags &= ~To<uint32_t>(EXTRA_RENDER_PASS::OUTLINE);

    m_tComponents.animator->OnAnimationFinished.Add_Listener(
        &CScoutBehavior_RequestResupply::On_AnimFinished, this);

    /* ERASE_강제_시작 */
    Process_Start();
}

void CScoutBehavior_RequestResupply::Process_Start()
{
    m_eState = RESUPPLY_STATE::DETECT;

    // TODO: 컷씬 / UI / 파티클
    LOG_INFO("Start CScoutBehavior_RequestResupply");
}

void CScoutBehavior_RequestResupply::Priority_Update(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);
}

void CScoutBehavior_RequestResupply::Update(_float fDT)
{
    /* fsm(sequence) */
    switch (m_eState)
    {
    case RESUPPLY_STATE::NONE:
        return;

    case RESUPPLY_STATE::DETECT:
        Process_DetectPlayer(fDT);
        break;

    case RESUPPLY_STATE::MOVE:
        Process_MoveBehindPlayer(fDT);
        break;

    case RESUPPLY_STATE::RESUPPLY:
        Process_Resupply();
        break;

    case RESUPPLY_STATE::FINISH:
        Process_Finish(fDT);
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

void CScoutBehavior_RequestResupply::OnTriggerEnter(const COLLISION_DESC& tCollisionDesc)
{
    if (m_eState != RESUPPLY_STATE::DETECT)
        return;

    CGameObject* pOther = GAME_INSTANCE.Find_GameObject(tCollisionDesc.hObject);
    if (!pOther)
        return;

    if (!Is_Player(pOther))
        return;

    On_DetectedPlayer(true, pOther);
    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::EMOTE_WAVE);
}

void CScoutBehavior_RequestResupply::OnTriggerExit(const COLLISION_DESC& tCollisionDesc)
{
    CGameObject* pOther = GAME_INSTANCE.Find_GameObject(tCollisionDesc.hObject);
    if (!pOther)
        return;

    if (pOther != m_goDetectedPlayer)
        return;

    On_DetectedPlayer(false, nullptr);
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

    if (!m_mr.Is_Valid())
        return;

    /* 플레이어 접근 시 외곽선 효과 */
    if (bDetected)
    {
        m_mr->extraPassFlags |= To<uint32_t>(EXTRA_RENDER_PASS::OUTLINE);

        auto* pBlock = GAME_INSTANCE.Get_PerObjectParamBlock(m_mr->hPerObjectParams);
        if (!pBlock)
            return;

        pBlock->block.Set_Float("g_OutlineWidth", m_fOutlineWidth);
        pBlock->block.Set_Float4("g_OutlineColor", { 1.f, 1.f, 1.f, 1.f });
    }
    else
    {
        m_mr->extraPassFlags &= ~To<uint32_t>(EXTRA_RENDER_PASS::OUTLINE);
    }
}

void CScoutBehavior_RequestResupply::Process_DetectPlayer(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);

    if (!m_goDetectedPlayer)
        return;

    CTransform trPlayer = m_goDetectedPlayer->Get_Component<CTransform>();

    _vector vPlayerPos = XMLoadFloat3(&trPlayer->vPosition);
    _vector vCurPos = XMLoadFloat3(&m_tComponents.transform->vPosition);

    _float fDist = XMVectorGetX(vPlayerPos - vCurPos);

    if (fDist < m_fBehindDistance)
    {
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RUN);
        m_eState = RESUPPLY_STATE::MOVE;
    }
    else if (fDist < m_fStopAnimationDist)
    {
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::IDLE_CASUAL_M);
    }
}

void CScoutBehavior_RequestResupply::Process_MoveBehindPlayer(_float fDT)
{
    if (!m_goDetectedPlayer)
        return;

    CTransform trPlayer = m_goDetectedPlayer->Get_Component<CTransform>();

    _vector vPlayerPos = XMLoadFloat3(&trPlayer->vPosition);
    _vector vLook = XMVector3Normalize(trPlayer.Get_StateXM(STATE::LOOK));
    _vector vTargetPos = vPlayerPos + vLook * m_fResupplyDistance;

    _vector vDiff = vTargetPos - XMLoadFloat3(&m_tComponents.transform->vPosition);
    _vector vDir = XMVector3Normalize(XMVectorSetY(vDiff, 0.f));

    m_tComponents.transform.Translate(vDir * m_pStats->fCurSpeed * fDT, SPACE::WORLD);
    m_tComponents.transform.Look_At(-vDir);

    _float fDist = XMVectorGetX(XMVector3Length(vDiff));

    if (fDist < m_fResupplyDistance)
    {
        m_eState = RESUPPLY_STATE::RESUPPLY;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RESUPPLY);
    }
}

void CScoutBehavior_RequestResupply::Process_Resupply()
{
    if (!m_goDetectedPlayer)
        return;
}

void CScoutBehavior_RequestResupply::Process_Finish(_float fDT)
{
    _vector vExit = XMLoadFloat3(&m_vExitPos);

    m_tComponents.transform.Look_At(-vExit);

    _vector vDir = XMVector3Normalize(vExit - XMLoadFloat3(&m_tComponents.transform->vPosition));
    vDir = XMVectorSetY(vDir, 0.f);

    m_tComponents.transform.Translate(vDir * m_pStats->fMaxSpeed * fDT, SPACE::WORLD);

    _float fDist = XMVectorGetX(XMVector3Length(vExit - XMLoadFloat3(&m_tComponents.transform->vPosition)));

    if (fDist < 0.2f)
    {
        m_goScout->Set_Enable(false);
    }
}

void CScoutBehavior_RequestResupply::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (m_eState != RESUPPLY_STATE::RESUPPLY)
        return;

    _uint iIndex = tData.iAnimationClip;

    if (iIndex == m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::RESUPPLY))
    {
        CPlayer* pPlayer = m_goDetectedPlayer->Get_Script<CPlayer>();
        if (!pPlayer)
            return;

        pPlayer->Complete_Deliver_Supplies();

        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::SPECIAL_ARMIN);
    }
    else if (iIndex == m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::SPECIAL_ARMIN))
    {
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RUN);
        m_eState = RESUPPLY_STATE::FINISH;
    }
}

NS_END
