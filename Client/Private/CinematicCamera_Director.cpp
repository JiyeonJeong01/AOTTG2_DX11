#include "CinematicCamera_Director.h"
#include "CinematicSystem.h"


NS_BEGIN(Client)

void CCinematicCamera_Director::Awake(void* pCtx)
{
    CGameObject* goCinematic = GAME_INSTANCE.Find_GameObject(m_refCinematicCam.hObject);
    IF_NULL_RETURN_MSG_BREAK(goCinematic, , "goCinematic is nullptr");

    m_camCinematic = goCinematic->Get_Component<CCamera>();

    SYS_CINEMATIC.Load("siva_clip");
}

void CCinematicCamera_Director::Start(void* pCtx)
{
}

void CCinematicCamera_Director::Priority_Update(void* pCtx, _float fDT)
{
    if (SYS_INPUT.Get_KeyDown('T') && m_camCinematic.Is_Valid())
    {
        SYS_CINEMATIC.Play(m_camCinematic);
        m_bPlay = true;

    }
    if (m_bPlay)
        SYS_CINEMATIC.Update(fDT);
}

void CCinematicCamera_Director::Update(void* pCtx, _float fDT)
{
}

void CCinematicCamera_Director::Late_Update(void* pCtx, _float fDT)
{
}

NS_END;
