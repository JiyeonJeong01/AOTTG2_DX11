#include "CinematicCamera_Director.h"
#include "CinematicSystem.h"


NS_BEGIN(Client)

void CCinematicCamera_Director::Awake(void* pCtx)
{
    CGameObject* goCinematic = GAME_INSTANCE.Find_GameObject(m_refCinematicCam.hObject);
    IF_NULL_RETURN_MSG_BREAK(goCinematic, , "goCinematic is nullptr");

    m_camCinematic = goCinematic->Get_Component<CCamera>();
}

void CCinematicCamera_Director::Start(void* pCtx)
{
    _bool bSuccess = SYS_CINEMATIC.Load("Scout_RequestResupply");
    IF_TRUE_RETURN_MSG_BREAK(!bSuccess, , "Scout_RequestResupply load failed");
    bSuccess = SYS_CINEMATIC.Load("Scout_RescueDialogue");
    IF_TRUE_RETURN_MSG_BREAK(!bSuccess, , "Scout_RescueDialogue load failed");
}

void CCinematicCamera_Director::Priority_Update(void* pCtx, _float fDT)
{
    //if (SYS_INPUT.Get_KeyDown('T') && m_camCinematic.Is_Valid())
    //{
    //    SYS_CINEMATIC.Play("siva_clip", m_camCinematic);
    //    m_bPlay = true;

    //}
    SYS_CINEMATIC.Update(fDT);
}

void CCinematicCamera_Director::Update(void* pCtx, _float fDT)
{
}

void CCinematicCamera_Director::Late_Update(void* pCtx, _float fDT)
{
}

// cpp
void CCinematicCamera_Director::On_TestCinematicEvent(const CINEMATIC_EVENT_DATA& tEventData)
{
    LOG_INFO("Cinematic Event : %s", tEventData.strEventName.c_str());

    if (tEventData.strEventName == "siva")
    {
        LOG_INFO("Cinematic Event : %s", tEventData.strEventName.c_str());

    }
}

NS_END;
