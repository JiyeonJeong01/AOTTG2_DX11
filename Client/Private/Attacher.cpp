#include "Attacher.h"

#include "GameInstance.h"
#include "GameObject.h"
#include "Transform.h"

NS_BEGIN(Client)

CAttacher::CAttacher()
{
}

CAttacher::~CAttacher()
{
}

void CAttacher::Awake(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    m_pOwner = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(m_pOwner, , "m_pOwner is nullptr.");

    m_trOwner = m_pOwner->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(m_trOwner.Is_Valid() == false, , "m_trOwner is invalid.");

    Register_Attach(m_tAttachObject);
}

void CAttacher::Start(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);
}

void CAttacher::Priority_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);
}

void CAttacher::Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);
}

void CAttacher::Late_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);

    if (!m_pOwner || m_trOwner.Is_Valid() == false)
        return;

    Sync_Attach(m_tAttachObject);
}

CGameObject* CAttacher::Get_AttachObject() const
{
    return m_goAttach;
}

void CAttacher::Stop_Attach()
{
    m_bCanAttach = false;
}

void CAttacher::Start_Attach()
{
    m_bCanAttach = true;
}

void CAttacher::Register_Attach(ATTACH_NODE& tNode)
{
    if (!tNode.refObject.Is_Valid())
        return;

    if (tNode.szBoneName[0] == '\0')
        return;

    m_goAttach = GAME_INSTANCE.Find_GameObject(tNode.refObject.hObject);
    if (!m_goAttach)
        return;

    tNode.trChild = m_goAttach->Get_Component<CTransform>();
    if (tNode.trChild.Is_Valid() == false)
        return;

    if (!GAME_INSTANCE.Find_AttachBoneInfo(m_hObject, tNode.szBoneName, tNode.pAnimData, tNode.iBoneIndex))
        return;

    m_bCanAttach = true;
}

void CAttacher::Sync_Attach(ATTACH_NODE& tNode)
{
    if (!m_bCanAttach)
        return;

    if (!tNode.refObject.Is_Valid())
        return;

    if (!tNode.pAnimData)
        return;

    if (tNode.trChild.Is_Valid() == false)
        return;

    const auto& vecCombined = tNode.pAnimData->boneCombinedMatrices;
    if (tNode.iBoneIndex >= vecCombined.size())
        return;

    const _matrix matBoneWorld =
        XMLoadFloat4x4(&vecCombined[tNode.iBoneIndex]) *
        XMLoadFloat4x4(&m_trOwner->matWorld);

    const _vector vLocalOffset = XMVectorSet(tNode.vOffset.x, tNode.vOffset.y, tNode.vOffset.z, 0.f);
    const _vector vWorldOffset = XMVector3TransformNormal(vLocalOffset, matBoneWorld);

    _vector vWorldPos = matBoneWorld.r[3] + vWorldOffset;

    tNode.trChild.Set_Position(vWorldPos);
}

std::shared_ptr<CAttacher> CAttacher::Create()
{
    auto pInstance = std::make_shared<CAttacher>();
    return pInstance;
}

NS_END
