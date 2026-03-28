#include "Player_InputController.h"
#include "Input_System.h"
#include "CRender_System.h"
#include "Render_Context.h"

CPlayer_InputController::CPlayer_InputController()
{
}

CPlayer_InputController::~CPlayer_InputController()
{
}

const PLAYER_INPUT_COMMAND& CPlayer_InputController::Update_InputCommand()
{
    /* 사용자의 입력을 받아 구조체를 채운다 */
    m_tInputCommand = {};

    /* 마우스 */
    long iMove = 0;
    if (iMove = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::HORIZONTAL))
        m_tInputCommand.vMouseDelta.x = To<_float>(iMove);
    if (iMove = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::VERTICAL))
        m_tInputCommand.vMouseDelta.y = To<_float>(iMove);
    if (iMove = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::DEPTH))
        m_tInputCommand.iSwitchSkillDir = iMove;

    /* 플레이어 이동 방향 */

    const _matrix matViewInv = Math::Load(SYS_RENDER.Contexts()->Get_ViewInv());

    _float3 vCamRight{};
    _float3 vCamLook{};

    Math::Store(vCamRight, matViewInv.r[0]);
    Math::Store(vCamLook, matViewInv.r[2]);

    vCamRight.y = 0.f;
    vCamLook.y = 0.f;

    _vector vRight = Math::Load(vCamRight);
    _vector vLook = Math::Load(vCamLook);

    const _float fEps = 1e-6f;

    /* 축 안정화 */
    if (Math::Get_X(XMVector3LengthSq(vRight)) > fEps)
        vRight = XMVector3Normalize(vRight);
    else
        vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);

    if (Math::Get_X(XMVector3LengthSq(vLook)) > fEps)
        vLook = XMVector3Normalize(vLook);
    else
        vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);

    _vector vMove = XMVectorZero();

    if (SYS_INPUT.Get_Key('W'))
        vMove += vLook;
    else if (SYS_INPUT.Get_Key('S'))
        vMove -= vLook;

    if (SYS_INPUT.Get_Key('A'))
        vMove -= vRight;
    if (SYS_INPUT.Get_Key('D'))
        vMove += vRight;

    if (!XMVector3Equal(vMove, XMVectorZero()))
        XMStoreFloat3(&m_tInputCommand.vMove, XMVector3Normalize(vMove));

    if (SYS_INPUT.Get_KeyDown('Q'))
        m_tInputCommand.bLeftAnchorPressed = true;
    else if (SYS_INPUT.Get_Key('Q'))
        m_tInputCommand.bLeftAnchorHeld = true;
    if (SYS_INPUT.Get_KeyDown('E'))
        m_tInputCommand.bRightAnchorPressed = true;
    else if (SYS_INPUT.Get_Key('E'))
        m_tInputCommand.bRightAnchorHeld = true;

    if (SYS_INPUT.Get_KeyDown(VK_SHIFT))
        m_tInputCommand.bBoostPressed = true;
    if (SYS_INPUT.Get_Key(VK_SHIFT))
        m_tInputCommand.bBoostHeld = true;

    if (SYS_INPUT.Get_Key(VK_SPACE))
        m_tInputCommand.bRopeReelHeld = true;

    if (SYS_INPUT.Get_KeyDown(VK_LBUTTON))
        m_tInputCommand.bNormalAttackPressed = true;
    if (SYS_INPUT.Get_KeyDown(VK_RBUTTON))
        m_tInputCommand.bStrongAttackPressed = true;

    if (SYS_INPUT.Get_KeyDown('F'))
        m_tInputCommand.bNormalAttackPressed = true;
    if (SYS_INPUT.Get_KeyDown('R'))
        m_tInputCommand.bInteract = true;


    return m_tInputCommand;
}

std::unique_ptr<CPlayer_InputController> CPlayer_InputController::Create()
{
    return std::make_unique<CPlayer_InputController>();
}
