#include "CinematicCamera_Director.h"
#include "CinematicSystem.h"


NS_BEGIN(Client)

void CCinematicCamera_Director::Awake(void* pCtx)
{
    CGameObject* goCinematic = GAME_INSTANCE.Find_GameObject(m_refCinematicCam.hObject);
    IF_NULL_RETURN_MSG_BREAK(goCinematic, , "goCinematic is nullptr");

    m_camCinematic = goCinematic->Get_Component<CCamera>();

    _bool bSuccess = SYS_CINEMATIC.Load("Scout_RequestResupply");
    IF_TRUE_RETURN_MSG_BREAK(!bSuccess, , "Scout_RequestResupply load failed");
    bSuccess = SYS_CINEMATIC.Load("Scout_RescueDialogue");
    IF_TRUE_RETURN_MSG_BREAK(!bSuccess, , "Scout_RescueDialogue load failed");
    bSuccess = SYS_CINEMATIC.Load("opening");
    IF_TRUE_RETURN_MSG_BREAK(!bSuccess, , "opening load failed");
    bSuccess = SYS_CINEMATIC.Load("LiftUp");
    IF_TRUE_RETURN_MSG_BREAK(!bSuccess, , "LiftUp load failed");
    bSuccess = SYS_CINEMATIC.Load("ending");
    IF_TRUE_RETURN_MSG_BREAK(!bSuccess, , "LiftUp load failed");
}

void CCinematicCamera_Director::Start(void* pCtx)
{

}

void CCinematicCamera_Director::Priority_Update(void* pCtx, _float fDT)
{
    SYS_CINEMATIC.Update(fDT);
}

void CCinematicCamera_Director::Update(void* pCtx, _float fDT)
{
    if (SYS_INPUT.Get_KeyDown(VK_F3))
    {
        SYS_CINEMATIC.Play("ending", m_camCinematic);
    }
}

void CCinematicCamera_Director::Late_Update(void* pCtx, _float fDT)
{
}

NS_END;
