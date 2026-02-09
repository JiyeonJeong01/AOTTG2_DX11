#include "Scene_Manager.h"

#include "Core_System.h"
#include "Scene.h"

NS_BEGIN(Engine)

CScene_Manager::CScene_Manager()
{
}

CScene_Manager::~CScene_Manager()
{
}

HRESULT CScene_Manager::Change_Scene(_uint iNewSceneIndex, std::unique_ptr<CScene> pNewScene)
{
    if (nullptr != m_pCurrentScene)
        SYS_CORE.Clear_Resources(m_iCurrentSceneIndex);

    m_pCurrentScene = std::move(pNewScene);
    m_iCurrentSceneIndex = iNewSceneIndex;

    return S_OK;
}

void CScene_Manager::Update(_float fTimeDelta)
{
    if (nullptr != m_pCurrentScene)
        m_pCurrentScene->Update(fTimeDelta);
}

HRESULT CScene_Manager::Render()
{
    if (nullptr != m_pCurrentScene)
        m_pCurrentScene->Render();

    return S_OK;
}

std::unique_ptr<CScene_Manager> CScene_Manager::Create()
{
    return std::make_unique<CScene_Manager>();
}


NS_END
