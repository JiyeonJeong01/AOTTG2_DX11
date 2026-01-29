#include "Scene.h"

#include "GameInstance.h"

CScene::CScene(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);

    Safe_AddRef(m_pGameInstance);
}

HRESULT CScene::Initialize()
{
    return S_OK;
}

void CScene::Update(_float fTimeDelta)
{
}

HRESULT CScene::Render()
{
    return S_OK;
}

void CScene::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);

    Safe_Release(m_pGameInstance);
}
