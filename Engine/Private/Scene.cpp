#include "Scene.h"

#include "Core_System.h"

NS_BEGIN(Engine)

CScene::CScene(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : LABEL("Untitled")
    , m_pDevice{ pDevice }
    , m_pContext{ pContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
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
}

NS_END
