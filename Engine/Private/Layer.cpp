#include "Layer.h"
#include "GameObject.h"

CLayer::CLayer()
{
}

HRESULT CLayer::Add_GameObject(CGameObject* pGameObject)
{
    if (!pGameObject)
    {
        _DEBUG_ERROR_BREAK("pGameObject is nullptr!");
        return E_FAIL;
    }
    m_GameObjects.push_back(pGameObject);
    return S_OK;
}

CLayer* CLayer::Create()
{
    return new CLayer();
}

void CLayer::Free()
{
    CBase::Free();
}
