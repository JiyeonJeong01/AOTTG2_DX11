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

    CMeshRenderer mr = m_goScout->Get_Component<CMeshRenderer>();
    IF_TRUE_RETURN_MSG_BREAK(!mr.Is_Valid(), , "mr is invalid");

    if (mr->hPerObjectParams == INVALID_HANDLE_UINT)
        mr->hPerObjectParams = GAME_INSTANCE.Alloc_PerObjectParamBlock();

    PER_OBJECT_PARAM_BLOCK* pBlock = GAME_INSTANCE.Get_PerObjectParamBlock(mr->hPerObjectParams);
    IF_NULL_RETURN_MSG_BREAK(pBlock, , "pBlock is nullptr");

    pBlock->block.Set_Float("g_OutlineWidth", m_fOutlineWidth);
    pBlock->block.Set_Float4("g_OutlineColor", { 1.f, 1.f, 1.f, 1.f });

    mr->extraPassFlags &= ~To<uint32_t>(EXTRA_RENDER_PASS::OUTLINE);
}

void CScoutBehavior_RequestResupply::Priority_Update(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);
}

void CScoutBehavior_RequestResupply::Update(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);

    if (!m_bPlayerDetected)
        return;

    Try_Interact();
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
    CGameObject* pOther = GAME_INSTANCE.Find_GameObject(tCollisionDesc.hObject);
    if (!pOther)
        return;

    if (!Is_Player(pOther))
        return;

    On_DetectedPlayer(true, pOther);
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
    if (pOther == nullptr)
        return false;

    return pOther->Has_Mask(O_PLAYER);
}

void CScoutBehavior_RequestResupply::On_DetectedPlayer(_bool bDetected, CGameObject* pPlayer)
{
    m_bPlayerDetected = bDetected;
    m_goDetectedPlayer = pPlayer;

    CMeshRenderer mr = m_goScout->Get_Component<CMeshRenderer>();
    if (!mr.Is_Valid())
        return;

    if (bDetected)
    {
        mr->extraPassFlags |= To<uint32_t>(EXTRA_RENDER_PASS::OUTLINE);

        auto* pBlock = GAME_INSTANCE.Get_PerObjectParamBlock(mr->hPerObjectParams);
        if (!pBlock)
            return;

        pBlock->block.Set_Float("g_OutlineWidth", m_fOutlineWidth);
        pBlock->block.Set_Float4("g_OutlineColor", { 1.f, 1.f, 1.f, 1.f });
    }
    else
    {
        mr->extraPassFlags &= ~To<uint32_t>(EXTRA_RENDER_PASS::OUTLINE);
    }
}

void CScoutBehavior_RequestResupply::Try_Interact()
{
    if (!m_bPlayerDetected)
        return;

    if (m_goDetectedPlayer == nullptr)
        return;

    if (!SYS_INPUT.Get_KeyDown('F'))
        return;

    CPlayer* pPlayer = m_goDetectedPlayer->Get_Script<CPlayer>();
    if (pPlayer == nullptr)
        return;

    pPlayer->Deliver_Supplies();

    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RESUPPLY);
}

NS_END
