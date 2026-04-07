#include "Environment_Controller.h"

NS_BEGIN(Client)

CEnvironment_Controller::CEnvironment_Controller()
{
}

CEnvironment_Controller::~CEnvironment_Controller()
{
}

void CEnvironment_Controller::Awake(void* pCtx)
{
    CGameObject* pObj = GAME_INSTANCE.Find_GameObject(m_refRainy.hObject);
    if (!pObj) return;

    m_mrRainy = pObj->Get_Component<CMeshRenderer>();
    if (!m_mrRainy.Is_Valid()) return;

    m_mrRainy.Set_ParticlePlaying(true);
}

void CEnvironment_Controller::Start(void* pCtx)
{
}

void CEnvironment_Controller::Priority_Update(void* pCtx, _float fDT)
{
}

void CEnvironment_Controller::Update(void* pCtx, _float fDT)
{
}

void CEnvironment_Controller::Late_Update(void* pCtx, _float fDT)
{
}

NS_END;
