#include "ResupplyStation.h"

#include "Player.h"

NS_BEGIN(Client)
    CResupplyStation::CResupplyStation()
{
}

CResupplyStation::~CResupplyStation()
{
}

void CResupplyStation::Awake(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    SetUp_References();
}

void CResupplyStation::Start(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    m_bPlayerDetected = false;
    m_goDetectedPlayer = nullptr;
    Set_UIVisible(false);

    CMeshRenderer mr = m_goOwner->Get_Component<CMeshRenderer>();
    IF_TRUE_RETURN_MSG_BREAK(!mr.Is_Valid(), , "mr is invalid");

    if (mr->hPerObjectParams == INVALID_HANDLE_UINT)
        mr->hPerObjectParams = GAME_INSTANCE.Alloc_PerObjectParamBlock();

    PER_OBJECT_PARAM_BLOCK* pBlock = GAME_INSTANCE.Get_PerObjectParamBlock(mr->hPerObjectParams);
    IF_NULL_RETURN_MSG_BREAK(pBlock, , "pBlock is nullptr");

    pBlock->block.Set_Float("g_OutlineWidth", m_fOutlineWidth);
    pBlock->block.Set_Float4("g_OutlineColor", { 1.f, 1.f, 0.f, 1.f });

    mr->extraPassFlags &= ~To<uint32_t>(EXTRA_RENDER_PASS::OUTLINE);

    m_eSide = SIDE::LEFT;
    m_vUILeftBaseSize = m_vUIRightBaseSize = m_vBaseSize;

    Apply_UISelectionVisual();
}

void CResupplyStation::Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);

    if (!m_bPlayerDetected)
        return;

    Update_UISelection();
    Try_Interact();
}

void CResupplyStation::Late_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);
}

_float3 CResupplyStation::Get_UIWorldPosition() const
{
    if (!m_trUIPivot.Is_Valid())
        return _float3(0.f, 0.f, 0.f);

    _float3 vWorldPos = m_trUIPivot->vPosition;
    vWorldPos.y += m_fUIHeightOffset;
    return vWorldPos;
}
void CResupplyStation::OnTriggerEnter(const COLLISION_DESC& tCollisionDesc)
{
    CGameObject* pOther = GAME_INSTANCE.Find_GameObject(tCollisionDesc.hObject);
    if (!pOther)
        return;

    if (!Is_Player(pOther))
        return;

    On_DetectedPlayer(true, pOther);
}

void CResupplyStation::OnTriggerExit(const COLLISION_DESC& tCollisionDesc)
{
    CGameObject* pOther = GAME_INSTANCE.Find_GameObject(tCollisionDesc.hObject);
    if (!pOther)
        return;

    if (pOther != m_goDetectedPlayer)
        return;

    On_DetectedPlayer(false, nullptr);
}

HRESULT CResupplyStation::SetUp_References()
{
    m_goOwner = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goOwner, E_FAIL, "m_goOwner is nullptr");

    CCollider trigger = m_goOwner->Get_Component<CCollider>();
    IF_TRUE_RETURN_MSG_BREAK(!trigger.Is_Valid(), E_FAIL, "trigger is invalid");

    trigger->OnTriggerEnter.Add_Listener(&CResupplyStation::OnTriggerEnter, this);
    trigger->OnTriggerExit.Add_Listener(&CResupplyStation::OnTriggerExit, this);

    m_trUIPivot = m_goOwner->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(!m_trUIPivot.Is_Valid(), E_FAIL, "m_trUIPivot is invalid");

    m_goUILeftObject = GAME_INSTANCE.Find_GameObject(m_refUILeftObject.hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goUILeftObject, E_FAIL, "m_goUILeftObject is nullptr");

    m_goUIRightObject = GAME_INSTANCE.Find_GameObject(m_refUIRightObject.hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goUIRightObject, E_FAIL, "m_goUIRightObject is nullptr");

    m_goUILeftTextObject = GAME_INSTANCE.Find_GameObject(m_refUITextLeftObject.hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goUILeftTextObject, E_FAIL, "goLeftText is nullptr");

    m_goUIRightTextObject = GAME_INSTANCE.Find_GameObject(m_refUITextRightObject.hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goUIRightTextObject, E_FAIL, "goRightText is nullptr");

    m_rtUILeft = m_goUILeftObject->Get_Component<CRectTransform>();
    IF_TRUE_RETURN_MSG_BREAK(!m_rtUILeft.Is_Valid(), E_FAIL, "m_rtUILeft is invalid");

    m_rtUIRight = m_goUIRightObject->Get_Component<CRectTransform>();
    IF_TRUE_RETURN_MSG_BREAK(!m_rtUIRight.Is_Valid(), E_FAIL, "m_rtUIRight is invalid");

    m_rtUITextLeft = m_goUILeftTextObject->Get_Component<CRectTransform>();
    IF_TRUE_RETURN_MSG_BREAK(!m_rtUITextLeft.Is_Valid(), E_FAIL, "m_rtUITextLeft is invalid");

    m_rtUITextRight = m_goUIRightTextObject->Get_Component<CRectTransform>();
    IF_TRUE_RETURN_MSG_BREAK(!m_rtUITextRight.Is_Valid(), E_FAIL, "m_rtUITextRight is invalid");

    return S_OK;
}

_bool CResupplyStation::Is_Player(const CGameObject* pOther) const
{
    if (pOther == nullptr)
        return false;

    return pOther->Has_Mask(O_PLAYER);
}

void CResupplyStation::On_DetectedPlayer(_bool bDetected, CGameObject* pPlayer)
{
    m_bPlayerDetected = bDetected;
    m_goDetectedPlayer = pPlayer;

    CMeshRenderer mr = m_goOwner->Get_Component<CMeshRenderer>();
    if (mr.Is_Valid())
    {
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

    if (bDetected)
    {
        m_bUpdatedPostition = false;
        Update_UIPosition();
        m_eSide = SIDE::LEFT;
        Apply_UISelectionVisual();
    }
    else
    {
        m_bUpdatedPostition = false;
        Set_UIVisible(false);
    }
}

void CResupplyStation::Update_UIPosition()
{
    _float2 vScreenPos = { 0.f, 0.f };
    if (!Project_WorldToScreen(Get_UIWorldPosition(), vScreenPos))
    {
        Set_UIVisible(false);
        return;
    }

    Set_UIVisible(true);

    const _float fUIY = vScreenPos.y - m_fUIHeightOffset;
    const _float fHalfSpacing = m_fUIHalfSpacing;

    m_rtUILeft.Set_PositionPx(vScreenPos.x - fHalfSpacing, fUIY);
    m_rtUIRight.Set_PositionPx(vScreenPos.x + fHalfSpacing, fUIY);
    m_rtUITextLeft.Set_PositionPx(vScreenPos.x - fHalfSpacing, fUIY);
    m_rtUITextRight.Set_PositionPx(vScreenPos.x + fHalfSpacing, fUIY);

    m_bUpdatedPostition = true;
}

void CResupplyStation::Update_UISelection()
{
    _bool bChanged = false;

    if (SYS_INPUT.Get_KeyDown(VK_LEFT))
    {
        m_eSide = SIDE::LEFT;
        bChanged = true;
    }
    else if (SYS_INPUT.Get_KeyDown(VK_RIGHT))
    {
        m_eSide = SIDE::RIGHT;
        bChanged = true;
    }

    if (bChanged)
        Apply_UISelectionVisual();
}

void CResupplyStation::Apply_UISelectionVisual()
{
    const _bool bLeftSelected = (m_eSide == SIDE::LEFT);
    const _bool bRightSelected = (m_eSide == SIDE::RIGHT);

    const _float fLeftUIScale = bLeftSelected ? m_fSelectedUIScale : m_fUnselectedUIScale;
    const _float fRightUIScale = bRightSelected ? m_fSelectedUIScale : m_fUnselectedUIScale;

    const _float fLeftTextScale = bLeftSelected ? m_fSelectedTextScale : m_fUnselectedTextScale;
    const _float fRightTextScale = bRightSelected ? m_fSelectedTextScale : m_fUnselectedTextScale;

    m_rtUILeft.Set_SizePx(
        m_vUILeftBaseSize.x * fLeftUIScale,
        m_vUILeftBaseSize.y * fLeftUIScale);

    m_rtUIRight.Set_SizePx(
        m_vUIRightBaseSize.x * fRightUIScale,
        m_vUIRightBaseSize.y * fRightUIScale);

    CUIText leftText = m_goUILeftTextObject->Get_Component<CUIText>();
    if (leftText.Is_Valid())
        leftText.Set_Scale(fLeftTextScale);

    CUIText rightText = m_goUIRightTextObject->Get_Component<CUIText>();
    if (rightText.Is_Valid())
        rightText.Set_Scale(fRightTextScale);
}

void CResupplyStation::Try_Interact()
{
    if (!m_bPlayerDetected)
        return;

    if (m_goDetectedPlayer == nullptr)
        return;

    if (!SYS_INPUT.Get_KeyDown(VK_RETURN))
        return;

    CPlayer* pPlayer = m_goDetectedPlayer->Get_Script<CPlayer>();
    if (pPlayer == nullptr)
        return;

    switch (m_eSide)
    {
    case SIDE::LEFT:
        pPlayer->Resupply();
        break;

    case SIDE::RIGHT:
        pPlayer->Ready_Deliver_Supplies();
        break;
    }
}

void CResupplyStation::Set_UIVisible(_bool bVisible)
{
    if (m_goUILeftObject != nullptr)
        m_goUILeftObject->Set_Enable(bVisible);
    if (m_goUILeftTextObject != nullptr)
        m_goUILeftTextObject->Set_Enable(bVisible);

    if (m_goUIRightObject != nullptr)
        m_goUIRightObject->Set_Enable(bVisible);
    if (m_goUIRightTextObject != nullptr)
        m_goUIRightTextObject->Set_Enable(bVisible);
}

_bool CResupplyStation::Project_WorldToScreen(const _float3& vWorldPos, _float2& vOutScreenPos) const
{
    const _float4x4 view4x4 = GAME_INSTANCE.Get_View();
    const _float4x4 proj4x4 = GAME_INSTANCE.Get_Proj();
    const _matrix matView = XMLoadFloat4x4(&view4x4);
    const _matrix matProj = XMLoadFloat4x4(&proj4x4);
    const D3D11_VIEWPORT tViewport = GAME_INSTANCE.Get_Viewport();

    const _vector vWorld = XMLoadFloat3(&vWorldPos);

    const XMVECTOR vProjected = XMVector3Project(
        vWorld,
        tViewport.TopLeftX,
        tViewport.TopLeftY,
        tViewport.Width,
        tViewport.Height,
        tViewport.MinDepth,
        tViewport.MaxDepth,
        matProj,
        matView,
        XMMatrixIdentity()
    );

    const _float fZ = XMVectorGetZ(vProjected);
    if (fZ < 0.f || fZ > 1.f)
        return false;

    vOutScreenPos.x = XMVectorGetX(vProjected);
    vOutScreenPos.y = XMVectorGetY(vProjected);

    return true;
}

NS_END
