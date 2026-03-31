#include "UI_SkillController.h"

#include "Player.h"
#include "Player_SkillController.h"

#include "Input_System.h"

NS_BEGIN(Client)

void CUI_SkillController::Awake(void* pCtx)
{
    /* GUID -> 런타임 텍스처 핸들 변환 */
    {
        if (m_SpinH_Texture.Is_Valid())
            m_hSpinH = GAME_INSTANCE.Get_ResourceHandle(ASSET_TYPE::TEXTURE, m_SpinH_Texture);
        if (m_Throw_Texture.Is_Valid())
            m_hThrow = GAME_INSTANCE.Get_ResourceHandle(ASSET_TYPE::TEXTURE, m_Throw_Texture);
        if (m_SpinV_Texture.Is_Valid())
            m_hSpinV = GAME_INSTANCE.Get_ResourceHandle(ASSET_TYPE::TEXTURE, m_SpinV_Texture);
        if (m_CoolDownSmall.Is_Valid())
            m_hCoolDownSmall = GAME_INSTANCE.Get_ResourceHandle(ASSET_TYPE::TEXTURE, m_CoolDownSmall);
        if (m_CoolDownBig.Is_Valid())
            m_hCoolDownBig = GAME_INSTANCE.Get_ResourceHandle(ASSET_TYPE::TEXTURE, m_CoolDownBig);
    }
}

void CUI_SkillController::Start(void* pCtx)
{

    /* skill controller 찾기 */
    {
        CGameObject* pPlayer = GAME_INSTANCE.Find_GameObject(m_refPlayer.hObject);
        IF_NULL_RETURN_MSG_BREAK(pPlayer, , "pPlayer is nullptr");

        CPlayer* scPlayer = pPlayer->Get_Script_InChildren<CPlayer>();
        IF_NULL_RETURN_MSG_BREAK(scPlayer, , "scPlayer is nullptr");

        m_pSkillController = scPlayer->Get_PlayerContext().pSkillController;
        IF_NULL_RETURN_MSG_BREAK(m_pSkillController, , "m_pSkillController is nullptr");

        m_pCurSkill = m_pSkillController->Get_CurSkillPtr();
    }

    /* 멤버 변수 할당 */
    {
        CGameObject* pObj = nullptr;
        pObj = GAME_INSTANCE.Find_GameObject(m_tRefSpinH_CoolDown_Image.hObject);
        IF_NULL_RETURN_MSG_BREAK(pObj, , "pObj is nullptr");
        m_SpinH_CoolDown_RT = pObj->Get_Component<CRectTransform>();
        IF_TRUE_RETURN_MSG_BREAK(!m_SpinH_CoolDown_RT.Is_Valid(), , "rect transform is invalid");

        pObj = GAME_INSTANCE.Find_GameObject(m_tRefThrow_CoolDown_Image.hObject);
        IF_NULL_RETURN_MSG_BREAK(pObj, , "pObj is nullptr");
        m_Throw_CoolDown_RT = pObj->Get_Component<CRectTransform>();
        IF_TRUE_RETURN_MSG_BREAK(!m_Throw_CoolDown_RT.Is_Valid(), , "rect transform is invalid");

        pObj = GAME_INSTANCE.Find_GameObject(m_tRefSpinV_CoolDown_Image.hObject);
        IF_NULL_RETURN_MSG_BREAK(pObj, , "pObj is nullptr");
        m_SpinV_CoolDown_RT = pObj->Get_Component<CRectTransform>();
        IF_TRUE_RETURN_MSG_BREAK(!m_SpinV_CoolDown_RT.Is_Valid(), , "rect transform is invalid");

        pObj = GAME_INSTANCE.Find_GameObject(m_tRefSkill_Image.hObject);
        IF_NULL_RETURN_MSG_BREAK(pObj, , "pObj is nullptr");
        m_Skill_Img = pObj->Get_Component<CUIImage>();
        IF_TRUE_RETURN_MSG_BREAK(!m_Skill_Img.Is_Valid(), , "image is invalid");

        pObj = GAME_INSTANCE.Find_GameObject(m_tRefSpinH_CoolDown_Image.hObject);
        IF_NULL_RETURN_MSG_BREAK(pObj, , "pObj is nullptr");
        m_crSpinHCoolDown = pObj->Get_Component<CCanvasRenderer>();
        IF_TRUE_RETURN_MSG_BREAK(!m_crSpinHCoolDown.Is_Valid(), , "image is invalid");

        pObj = GAME_INSTANCE.Find_GameObject(m_tRefThrow_CoolDown_Image.hObject);
        IF_NULL_RETURN_MSG_BREAK(pObj, , "pObj is nullptr");
        m_crThrowCoolDown = pObj->Get_Component<CCanvasRenderer>();
        IF_TRUE_RETURN_MSG_BREAK(!m_crThrowCoolDown.Is_Valid(), , "image is invalid");

        pObj = GAME_INSTANCE.Find_GameObject(m_tRefSpinV_CoolDown_Image.hObject);
        IF_NULL_RETURN_MSG_BREAK(pObj, , "pObj is nullptr");
        m_crSpinVCoolDown = pObj->Get_Component<CCanvasRenderer>();
        IF_TRUE_RETURN_MSG_BREAK(!m_crSpinVCoolDown.Is_Valid(), , "image is invalid");
    }

    m_crSpinHCoolDown.Enable_ClipRect(true);
    m_crThrowCoolDown.Enable_ClipRect(true);
    m_crSpinVCoolDown.Enable_ClipRect(true);

    /* 이벤트 등록 */
    {
        m_pSkillController->Subscribe_OnActivatedSkillChanged(&CUI_SkillController::On_ActiveSkillChanged, this);
    }

    Refresh_AllCoolDownUI();
}

void CUI_SkillController::Priority_Update(void* pCtx, _float fDT)
{
}

void CUI_SkillController::Update(void* pCtx, _float fDT)
{
    if (SYS_INPUT.Get_KeyDown('I'))
        m_pSkillController->Try_UseSKill(SKILL_TYPE::SPIN_H);
    if (SYS_INPUT.Get_KeyDown('O'))
        m_pSkillController->Try_UseSKill(SKILL_TYPE::THROW);
    if (SYS_INPUT.Get_KeyDown('P'))
        m_pSkillController->Try_UseSKill(SKILL_TYPE::SPIN_V);
}

void CUI_SkillController::Late_Update(void* pCtx, _float fDT)
{
    Refresh_AllCoolDownUI();
}

void CUI_SkillController::Apply_CoolDownState(CCanvasRenderer& tImg, CRectTransform& tRT, SKILL_TYPE eTargetSkillType, SKILL_TYPE eActiveSkillType)
{
    if (!m_pSkillController)
        return;

    const _bool bIsActive = (eTargetSkillType == eActiveSkillType);

    tImg.Set_Texture(bIsActive ? m_hCoolDownBig : m_hCoolDownSmall);

    _float2 vRtPos = bIsActive ? m_vCoolDownBigPos[To<_uint>(eTargetSkillType)] : m_vCoolDownSmallPos[To<_uint>(eTargetSkillType)];
    _float2 vRtSize = bIsActive ? m_vCoolDownBigSize : m_vCoolDownSmallSize;

    if (eTargetSkillType == SKILL_TYPE::THROW
        && eActiveSkillType == SKILL_TYPE::SPIN_V)
        vRtPos.x = 1657.f;

    tRT.Set_PositionPx(vRtPos.x, vRtPos.y);
    tRT.Set_SizePx(vRtSize.x, vRtSize.y);

    _float fElapsed = m_pSkillController->Get_ElapsedCoolDown(eTargetSkillType);
    _float fTotal = m_pSkillController->Get_TotalCoolDown(eTargetSkillType);

    if (fTotal <= 0.f)
    {
        tImg.Set_UV(RECT_F{ 0.f, 0.f, 0.f, 0.f });
        return;
    }

    _float fRatio = fElapsed / fTotal;
    fRatio = std::clamp(fRatio, 0.f, 1.f);

    /* Top을 증가시키면 위에서 아래로 줄어듦
       - 쿨다운 시작 : 거의 다 가려져야 함
       - 쿨다운 종료 : 안 가려져야 함 */
    const _float fRemainRatio = 1.f - fRatio;

    const _float2 vSize = bIsActive ? m_vCoolDownBigSize : m_vCoolDownSmallSize;
    const _float fTop = vSize.y * fRatio;

    tImg.Set_ClipRect(RECT_F{ 0.f, fTop, 0.f, 0.f });
}

void CUI_SkillController::Refresh_AllCoolDownUI()
{
    if (!m_pSkillController)
        return;

    /* 현재 활성화된 스킬 타입 */
    SKILL_TYPE eActiveSkillType = m_pSkillController->Get_CurSkillType();

    Apply_CoolDownState(m_crSpinHCoolDown, m_SpinH_CoolDown_RT, SKILL_TYPE::SPIN_H, eActiveSkillType);
    Apply_CoolDownState(m_crThrowCoolDown, m_Throw_CoolDown_RT, SKILL_TYPE::THROW, eActiveSkillType);
    Apply_CoolDownState(m_crSpinVCoolDown, m_SpinV_CoolDown_RT, SKILL_TYPE::SPIN_V, eActiveSkillType);
}

void CUI_SkillController::On_ActiveSkillChanged(SKILL_TYPE eSkillType)
{
    switch (eSkillType)
    {
    case SKILL_TYPE::SPIN_H:
        m_Skill_Img.Set_Texture(m_hSpinH);
        break;

    case SKILL_TYPE::THROW:
        m_Skill_Img.Set_Texture(m_hThrow);
        break;

    case SKILL_TYPE::SPIN_V:
        m_Skill_Img.Set_Texture(m_hSpinV);
        break;
    }

    Refresh_AllCoolDownUI();
}

NS_END;
