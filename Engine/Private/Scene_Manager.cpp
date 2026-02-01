#include "Scene_Manager.h"

#include "GameInstance.h"
#include "Scene.h"

NS_BEGIN(Engine)

CScene_Manager::CScene_Manager()
    : m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CScene_Manager::Change_Scene(_uint iNewSceneIndex, CScene* pNewScene)
{
    if (nullptr != m_pCurrentScene)
        m_pGameInstance->Clear_Resources(m_iCurrentSceneIndex);

    if (0 != Safe_Release(m_pCurrentScene))
    {
        return E_FAIL;
    }

    m_pCurrentScene = pNewScene;

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

CScene_Manager* CScene_Manager::Create()
{
    return new CScene_Manager();
}

void CScene_Manager::Free()
{
    __super::Free();

    Safe_Release(m_pCurrentScene);
    Safe_Release(m_pGameInstance);
}

NS_END
