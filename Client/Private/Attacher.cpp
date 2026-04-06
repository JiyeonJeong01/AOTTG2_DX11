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

void CAttacher::Register_Attach(ATTACH_NODE& tNode)
{
    if (!tNode.refObject.Is_Valid())
        return;

    if (tNode.szBoneName[0] == '\0')
        return;

    Engine::CGameObject* pChildObject = GAME_INSTANCE.Find_GameObject(tNode.refObject.hObject);
    if (!pChildObject)
        return;

    tNode.trChild = pChildObject->Get_Component<CTransform>();
    if (tNode.trChild.Is_Valid() == false)
        return;

    if (!GAME_INSTANCE.Find_AttachBoneInfo(m_hObject, tNode.szBoneName, tNode.pAnimData, tNode.iBoneIndex))
        return;
}

void CAttacher::Sync_Attach(ATTACH_NODE& tNode)
{
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

    _vector vWorldPos = matBoneWorld.r[3];
    vWorldPos += XMVectorSet(tNode.vOffset.x, tNode.vOffset.y, tNode.vOffset.z, 0.f);

    tNode.trChild.Set_Position(vWorldPos);
}

std::shared_ptr<CAttacher> CAttacher::Create()
{
    auto pInstance = std::make_shared<CAttacher>();
    return pInstance;
}

NS_END
