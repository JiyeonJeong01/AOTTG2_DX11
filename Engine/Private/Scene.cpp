#include "Scene.h"

#include "Core_System.h"

NS_BEGIN(Engine)

CScene::CScene()
    : LABEL("Untitled")
, m_pDevice(nullptr), m_pContext(nullptr)
{
}

CScene::CScene(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : LABEL("Untitled")
    , m_pDevice{ pDevice }
    , m_pContext{ pContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

CScene::~CScene()
{
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
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

const ASSET_GUID& CScene::Get_GUID() const
{
    return m_tGUID;
}

void CScene::Set_GUID(const ASSET_GUID& tGUID)
{
    m_tGUID = tGUID;
}

SCENE_STATE CScene::Get_State() const
{
    return m_eState;
}

void CScene::Set_State(SCENE_STATE eState)
{
    m_eState = eState;
}

std::unique_ptr<CScene> CScene::Create()
{
    return make_unique<CScene>();
}

NS_END
