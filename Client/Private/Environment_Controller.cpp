#include "Environment_Controller.h"
#include "Resource_System.h"

NS_BEGIN(Client)

CEnvironment_Controller::CEnvironment_Controller()
{
}

CEnvironment_Controller::~CEnvironment_Controller()
{
}

void CEnvironment_Controller::Awake(void* pCtx)
{
    CGameObject* pObj = GAME_INSTANCE.Find_GameObject(m_refWater.hObject);
    if (!pObj) return;

    m_mrWater = pObj->Get_Component<CMeshRenderer>();
    if (!m_mrWater.Is_Valid()) return;

    m_hPerObjBlockWater = SYS_RESOURCE.Alloc_PerObjectParamBlock();
    m_mrWater->hPerObjectParams = m_hPerObjBlockWater;
    m_pBlockWater = SYS_RESOURCE.Get_PerObjectParamBlock(m_hPerObjBlockWater);
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
    if (m_pBlockWater == nullptr)
        return;

    m_fDT += fDT;
    if (m_fDT < 0.f) m_fDT = 0.f;
    m_pBlockWater->block.Set_Float("g_fTime", m_fDT);

}

NS_END;
