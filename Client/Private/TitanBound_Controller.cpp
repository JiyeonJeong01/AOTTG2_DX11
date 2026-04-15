#include "TitanBound_Controller.h"

#include "AnimationClip_Titan.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "Transform.h"

using namespace DirectX;

NS_BEGIN(Client)

namespace
{
    bool Is_Valid_ObjectRef(const SCRIPT_OBJECT_REF& refObj)
    {
        return refObj.Is_Valid();
    }
}

CTitanBound_Controller::CTitanBound_Controller()
{
}

CTitanBound_Controller::~CTitanBound_Controller()
{
}

void CTitanBound_Controller::Awake(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    m_pOwner = GAME_INSTANCE.Find_GameObject(m_refOwner.hObject);
    if (!m_pOwner)
        m_pOwner = GAME_INSTANCE.Find_GameObject(m_hObject);

    IF_NULL_RETURN_MSG_BREAK(m_pOwner, , "m_pOwner is nullptr.");

    m_trOwner = m_pOwner->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(m_trOwner.Is_Valid() == false, , "owner transform is invalid.");

    m_vecBounds.clear();
    m_vecBounds.reserve(19);

    /* Register */
    {
        Register_Bound(m_vecBounds, m_refGrabAirFarL, m_vGrabAirFarLOffset);
        Register_Bound(m_vecBounds, m_refGrabAirFarR, m_vGrabAirFarROffset);

        Register_Bound(m_vecBounds, m_refGrabAirShortL, m_vGrabAirShortLOffset);
        Register_Bound(m_vecBounds, m_refGrabAirShortR, m_vGrabAirShortROffset);

        Register_Bound(m_vecBounds, m_refGrabBackL, m_vGrabBackLOffset);
        Register_Bound(m_vecBounds, m_refGrabBackR, m_vGrabBackROffset);

        Register_Bound(m_vecBounds, m_refGrabGroundBackL, m_vGrabGroundBackLOffset);
        Register_Bound(m_vecBounds, m_refGrabGroundBackR, m_vGrabGroundBackROffset);

        Register_Bound(m_vecBounds, m_refGrabGroundFrontL, m_vGrabGroundFrontLOffset);
        Register_Bound(m_vecBounds, m_refGrabGroundFrontR, m_vGrabGroundFrontROffset);

        Register_Bound(m_vecBounds, m_refGrabHeadBackL, m_vGrabHeadBackLOffset);
        Register_Bound(m_vecBounds, m_refGrabHeadBackR, m_vGrabHeadBackROffset);

        Register_Bound(m_vecBounds, m_refGrabHeadFrontL, m_vGrabHeadFrontLOffset);
        Register_Bound(m_vecBounds, m_refGrabHeadFrontR, m_vGrabHeadFrontROffset);

        Register_Bound(m_vecBounds, m_refGrabHighL, m_vGrabHighLOffset);
        Register_Bound(m_vecBounds, m_refGrabHighR, m_vGrabHighROffset);

        Register_Bound(m_vecBounds, m_refGrabStomachL, m_vGrabStomachLOffset);
        Register_Bound(m_vecBounds, m_refGrabStomachR, m_vGrabStomachROffset);
    }

    /* Bind Trigger */
    {
        Bind_Trigger(m_refHandL, &CTitanBound_Controller::OnTriggerEnter_HandL);
        Bind_Trigger(m_refHandR, &CTitanBound_Controller::OnTriggerEnter_HandR);

        Bind_Trigger(m_refGrabAirFarL, &CTitanBound_Controller::OnTriggerEnter_GrabAirFarL);
        Bind_Trigger(m_refGrabAirFarR, &CTitanBound_Controller::OnTriggerEnter_GrabAirFarR);

        Bind_Trigger(m_refGrabAirShortL, &CTitanBound_Controller::OnTriggerEnter_GrabAirShortL);
        Bind_Trigger(m_refGrabAirShortR, &CTitanBound_Controller::OnTriggerEnter_GrabAirShortR);

        Bind_Trigger(m_refGrabBackL, &CTitanBound_Controller::OnTriggerEnter_GrabBackL);
        Bind_Trigger(m_refGrabBackR, &CTitanBound_Controller::OnTriggerEnter_GrabBackR);

        Bind_Trigger(m_refGrabGroundBackL, &CTitanBound_Controller::OnTriggerEnter_GrabGroundBackL);
        Bind_Trigger(m_refGrabGroundBackR, &CTitanBound_Controller::OnTriggerEnter_GrabGroundBackR);

        Bind_Trigger(m_refGrabGroundFrontL, &CTitanBound_Controller::OnTriggerEnter_GrabGroundFrontL);
        Bind_Trigger(m_refGrabGroundFrontR, &CTitanBound_Controller::OnTriggerEnter_GrabGroundFrontR);

        Bind_Trigger(m_refGrabHeadBackL, &CTitanBound_Controller::OnTriggerEnter_GrabHeadBackL);
        Bind_Trigger(m_refGrabHeadBackR, &CTitanBound_Controller::OnTriggerEnter_GrabHeadBackR);

        Bind_Trigger(m_refGrabHeadFrontL, &CTitanBound_Controller::OnTriggerEnter_GrabHeadFrontL);
        Bind_Trigger(m_refGrabHeadFrontR, &CTitanBound_Controller::OnTriggerEnter_GrabHeadFrontR);

        Bind_Trigger(m_refGrabHighL, &CTitanBound_Controller::OnTriggerEnter_GrabHighL);
        Bind_Trigger(m_refGrabHighR, &CTitanBound_Controller::OnTriggerEnter_GrabHighR);

        Bind_Trigger(m_refGrabStomachL, &CTitanBound_Controller::OnTriggerEnter_GrabStomachL);
        Bind_Trigger(m_refGrabStomachR, &CTitanBound_Controller::OnTriggerEnter_GrabStomachR);
    }

    /* Attach */
    {
        Attach(m_refHandL, m_szHandL, {0.f, 0.f, 0.f});
        Attach(m_refHandR, m_szHandR, { 0.f, 0.f, 0.f });

        Attach(m_refHitBoxHandL, m_szHitBoxHandL, {0.f, 0.f, 0.f});
        Attach(m_refHitBoxHandR, m_szHitBoxHandR, { 0.f, 0.f, 0.f });
        Attach(m_refHitBoxLegL, m_szHitBoxLegL, {0.f, 0.f, 0.f});
        Attach(m_refHitBoxLegR, m_szHitBoxLegR, { 0.f, 0.f, 0.f });

        Attach(m_refHurtBoxArmL, m_szHurtBoxArmL, {0.f, 0.f, 0.f});
        Attach(m_refHurtBoxArmR, m_szHurtBoxArmR, { 0.f, 0.f, 0.f });
        Attach(m_refHurtBoxLegL, m_szHurtBoxLegL, {0.f, 0.f, 0.f});
        Attach(m_refHurtBoxLegR, m_szHurtBoxLegR, { 0.f, 0.f, 0.f });
    }
}

void CTitanBound_Controller::Start(void* pCtx)
{
}

void CTitanBound_Controller::Priority_Update(void* pCtx, _float fDT)
{
}

void CTitanBound_Controller::Update(void* pCtx, _float fDT)
{
}

void CTitanBound_Controller::Late_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);

    if (!m_pOwner || m_trOwner.Is_Valid() == false)
        return;

    const _vector vOwnerPos = XMLoadFloat3(&m_trOwner->vPosition);
    const _vector vOwnerRot = XMLoadFloat4(&m_trOwner->vRotationQuat);

    for (auto& tAttach : m_vecAttachBones)
        Sync_AttchBone(tAttach);

    for (auto& tBound : m_vecBounds)
        Sync_Bound(tBound, vOwnerPos, vOwnerRot);
}

void CTitanBound_Controller::Attach(const SCRIPT_OBJECT_REF& refObject, const std::string& strBoneName, const _float3& vOffset)
{
    if (!Is_Valid_ObjectRef(refObject))
        return;

    if (!m_pOwner || m_trOwner.Is_Valid() == false)
        return;

    Engine::CGameObject* pChildObject = GAME_INSTANCE.Find_GameObject(refObject.hObject);
    if (!pChildObject)
        return;

    CTransform trChild = pChildObject->Get_Component<CTransform>();
    if (trChild.Is_Valid() == false)
        return;

    TITAN_ATTACH_BONE tAttach{};

    if (!GAME_INSTANCE.Find_AttachBoneInfo(m_hObject, strBoneName, tAttach.pAnimData, tAttach.iBoneIndex))
        return;

    tAttach.trParent = m_trOwner;
    tAttach.trChild = trChild;
    tAttach.vOffset = vOffset;

    m_vecAttachBones.push_back(tAttach);
}

void CTitanBound_Controller::Sync_AttchBone(TITAN_ATTACH_BONE& tAttach)
{
    if (!tAttach.pAnimData)
        return;

    if (tAttach.trParent.Is_Valid() == false || tAttach.trChild.Is_Valid() == false)
        return;

    const auto& combined = tAttach.pAnimData->boneCombinedMatrices;
    if (tAttach.iBoneIndex >= combined.size())
        return;

    const _matrix matWorld =
        XMLoadFloat4x4(&combined[tAttach.iBoneIndex]) *
        XMLoadFloat4x4(&tAttach.trParent->matWorld);

    _vector vWorldPos = matWorld.r[3];
    /* 직접 이동시키기 */
    vWorldPos += XMVectorSet(tAttach.vOffset.x, tAttach.vOffset.y, tAttach.vOffset.z, 0.f);

    tAttach.trChild.Set_Position(vWorldPos);
}

void CTitanBound_Controller::Register_Bound(std::vector<TITAN_BOUND_NODE>& vecBounds, SCRIPT_OBJECT_REF& refBound, _float3& vOffset)
{
    if (!Is_Valid_ObjectRef(refBound))
        return;

    Engine::CGameObject* pBound = GAME_INSTANCE.Find_GameObject(refBound.hObject);
    if (!pBound)
        return;

    CTransform trBound = pBound->Get_Component<CTransform>();
    if (trBound.Is_Valid() == false)
        return;

    TITAN_BOUND_NODE tNode{};
    tNode.refObject = refBound;
    tNode.pObject = pBound;
    tNode.trObject = trBound;
    tNode.pOffset = &vOffset;

    vecBounds.push_back(tNode);
}

void CTitanBound_Controller::Sync_Bound(TITAN_BOUND_NODE& tBound, _fvector vOwnerPos, _fvector vOwnerRot) const
{
    if (!tBound.pObject || tBound.trObject.Is_Valid() == false || !tBound.pOffset)
        return;

    const _vector vLocalOffset = XMLoadFloat3(tBound.pOffset);
    const _vector vWorldOffset = XMVector3Rotate(vLocalOffset, vOwnerRot);
    const _vector vWorldPos = vOwnerPos + vWorldOffset;

    XMStoreFloat3(&tBound.trObject->vPosition, vWorldPos);
    tBound.trObject->vRotationQuat = m_trOwner->vRotationQuat;
}

void CTitanBound_Controller::Bind_Trigger(
    const SCRIPT_OBJECT_REF& refBound,
    void (CTitanBound_Controller::* pFunc)(const COLLISION_DESC&))
{
    CGameObject* pBound = GAME_INSTANCE.Find_GameObject(refBound.hObject);
    if (!pBound)
        return;

    auto colBound = pBound->Get_Component<CCollider>();
    if (colBound.Is_Valid() == false)
        return;

    /* ERASE_마스크_설정 */
    //colBound->iDiscardMask = (O_EREN | O_ENEMY | O_HITBOX | O_HURTBOX | O_WALKABLE);

    colBound->OnTriggerEnter.Add_Listener(pFunc, this);
}

void CTitanBound_Controller::Try_QueueGrabAnim(const char* pAnimName, const COLLISION_DESC& tDesc)
{
    if (!m_bGrabTriggerEnabled)
        return;

    if (m_bPendingGrabAnim)
        return;

    CGameObject* pCounter = GAME_INSTANCE.Find_GameObject(tDesc.hObject);
    if (!pCounter)
        return;

    /* 플레이어 + HUMAN만 받기 */
    if (!pCounter->Is_ExactMask(O_SCOUT) && !pCounter->Is_ExactMask(O_PLAYER))
        return;

    m_strPendingGrabAnim = pAnimName;
    m_bPendingGrabAnim = true;
}

_bool CTitanBound_Controller::Consume_PendingGrabAnim(std::string& strOutAnim)
{
    if (!m_bPendingGrabAnim)
        return false;

    strOutAnim = m_strPendingGrabAnim;

    m_strPendingGrabAnim.clear();
    m_bPendingGrabAnim = false;
    return true;
}

void CTitanBound_Controller::Clear_PendingGrabAnim()
{
    m_strPendingGrabAnim.clear();
    m_bPendingGrabAnim = false;
}

void CTitanBound_Controller::Set_GrabTriggerEnabled(_bool bEnable)
{
    m_bGrabTriggerEnabled = bEnable;

    if (!bEnable)
        Clear_PendingGrabAnim();
}

CGameObject* CTitanBound_Controller::Get_GrabbedObject()
{
    return m_goGrabbed;
}

CHuman* CTitanBound_Controller::Get_GrabbedHuman()
{
    return m_pHuman;
}

_float3* CTitanBound_Controller::Get_GrabbedPoint()
{
    return m_pGrabbedPoint;
}

void CTitanBound_Controller::Handle_GrabState(SIDE eSide, CGameObject* pTarget, CGameObject* pHand)
{
    if (!pTarget->Is_ExactMask(O_SCOUT) && !pTarget->Is_ExactMask(O_PLAYER))
        return;

    CTransform tr = pHand->Get_Component<CTransform>();

    m_pGrabbedPoint = &tr._Data()->vPosition;

    /* class Player : public IScript, public CHuman 이므로 다중 상속 가능 */
    CHuman* pHuman = pTarget->Get_Script_InChildren<CHuman>();
    /* class NormalTitan : public IScript, public CTitan 이므로 다중 상속 가능 */
    CTitan* pTitan = m_pOwner->Get_Script<CTitan>();
    if (nullptr == pHuman || nullptr == pTitan)
        return;
    m_pHuman = pHuman;
    m_goGrabbed = pTarget;

    pTitan->On_Grab(eSide, pHuman);
}

void CTitanBound_Controller::OnTriggerEnter_HandL(const COLLISION_DESC& tDesc)
{
    CGameObject* pTarget = GAME_INSTANCE.Find_GameObject(tDesc.hObject);
    if (!pTarget)
        return;

    CGameObject* pLeftHand = GAME_INSTANCE.Find_GameObject(m_refHandL.hObject);
    if (!pLeftHand)
        return;

    Handle_GrabState(SIDE::LEFT, pTarget, pLeftHand);
}

void CTitanBound_Controller::OnTriggerEnter_HandR(const COLLISION_DESC& tDesc)
{
    CGameObject* pTarget = GAME_INSTANCE.Find_GameObject(tDesc.hObject);
    if (!pTarget)
        return;

    CGameObject* pRightHand = GAME_INSTANCE.Find_GameObject(m_refHandR.hObject);
    if (!pRightHand)
        return;

    Handle_GrabState(SIDE::RIGHT, pTarget, pRightHand);
}

void CTitanBound_Controller::OnTriggerEnter_GrabAirFarL(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_AIR_FAR_L, tDesc); }
void CTitanBound_Controller::OnTriggerEnter_GrabAirFarR(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_AIR_FAR_R, tDesc); }

void CTitanBound_Controller::OnTriggerEnter_GrabAirShortL(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_AIR_SHORT_L, tDesc); }
void CTitanBound_Controller::OnTriggerEnter_GrabAirShortR(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_AIR_SHORT_R, tDesc); }

void CTitanBound_Controller::OnTriggerEnter_GrabBackL(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_BACK_L, tDesc); }
void CTitanBound_Controller::OnTriggerEnter_GrabBackR(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_BACK_R, tDesc); }

void CTitanBound_Controller::OnTriggerEnter_GrabGroundBackL(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_GROUND_BACK_L, tDesc); }
void CTitanBound_Controller::OnTriggerEnter_GrabGroundBackR(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_GROUND_BACK_R, tDesc); }

void CTitanBound_Controller::OnTriggerEnter_GrabGroundFrontL(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_GROUND_FRONT_L, tDesc); }
void CTitanBound_Controller::OnTriggerEnter_GrabGroundFrontR(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_GROUND_FRONT_R, tDesc); }

void CTitanBound_Controller::OnTriggerEnter_GrabHeadBackL(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_HEAD_BACK_L, tDesc); }
void CTitanBound_Controller::OnTriggerEnter_GrabHeadBackR(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_HEAD_BACK_R, tDesc); }

void CTitanBound_Controller::OnTriggerEnter_GrabHeadFrontL(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_HEAD_FRONT_1, tDesc); }
void CTitanBound_Controller::OnTriggerEnter_GrabHeadFrontR(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_HEAD_FRONT_R, tDesc); }

void CTitanBound_Controller::OnTriggerEnter_GrabHighL(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_HIGH_L, tDesc); }
void CTitanBound_Controller::OnTriggerEnter_GrabHighR(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_HIGH_R, tDesc); }

void CTitanBound_Controller::OnTriggerEnter_GrabStomachL(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_STOMACH_L, tDesc); }
void CTitanBound_Controller::OnTriggerEnter_GrabStomachR(const COLLISION_DESC& tDesc) { Try_QueueGrabAnim(ANIM_TITAN::GRAB_STOMACH_R, tDesc); }

NS_END
