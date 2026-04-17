#include "Graphic_Device.h"
#include "Engine_Log.h"

NS_BEGIN(Engine)

CGraphic_Device::CGraphic_Device()
	: m_pDevice{ nullptr }, m_pDeviceContext{ nullptr }
{
}

CGraphic_Device::~CGraphic_Device()
{
    Safe_Release(m_pSwapChain);
    Safe_Release(m_pDefaultDSV);
    Safe_Release(m_pDefaultRTV);
    Safe_Release(m_pDeviceContext);
    Safe_Release(m_pSceneTexture);
    Safe_Release(m_pSceneRTV);
    Safe_Release(m_pSceneSRV);
    Safe_Release(m_pSceneDSV);
    Safe_Release(m_pSceneDepthSRV);

    Safe_Release(m_pDiffuseTexture);
    Safe_Release(m_pDiffuseRTV);
    Safe_Release(m_pDiffuseSRV);
    Safe_Release(m_pNormalTexture);
    Safe_Release(m_pNormalRTV);
    Safe_Release(m_pNormalSRV);
    Safe_Release(m_pLightTexture);
    Safe_Release(m_pLightRTV);
    Safe_Release(m_pLightSRV);

#if defined(DEBUG) || defined(_DEBUG)
    ID3D11Debug* d3dDebug;
    HRESULT hr = m_pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
    if (SUCCEEDED(hr))
    {
        OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");
        OutputDebugStringW(L"                                                                    D3D11 Live Object ref Count Checker \r ");
        OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");

        hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

        OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");
        OutputDebugStringW(L"                                                                    D3D11 Live Object ref Count Checker END \r ");
        OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");
    }
    if (d3dDebug != nullptr)            d3dDebug->Release();
#endif


    Safe_Release(m_pDevice);
}

HRESULT CGraphic_Device::Initialize(HWND hWnd, WINMODE isWindowed, _uint iWinSizeX, _uint iWinSizeY, _Out_ ID3D11Device** ppDevice, _Out_ ID3D11DeviceContext** ppContext)
{
	_uint		iFlag = 0;

#ifdef _DEBUG
	iFlag = D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL			FeatureLV;

	if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, iFlag, nullptr, 0, D3D11_SDK_VERSION, &m_pDevice, &FeatureLV, &m_pDeviceContext)))
		return E_FAIL;

	if (FAILED(Ready_SwapChain(hWnd, isWindowed, iWinSizeX, iWinSizeY)))
		return E_FAIL;

	if (FAILED(Ready_Default_RTV()))
		return E_FAIL;

	if (FAILED(Ready_Default_DSV(iWinSizeX, iWinSizeY)))
		return E_FAIL;

	ID3D11RenderTargetView* pRTVs[] = { m_pDefaultRTV, };

    m_pDeviceContext->OMSetRenderTargets(1, pRTVs, m_pDefaultDSV);

	D3D11_VIEWPORT			ViewPortDesc;
	ZeroMemory(&ViewPortDesc, sizeof(D3D11_VIEWPORT));
	ViewPortDesc.TopLeftX = 0;
	ViewPortDesc.TopLeftY = 0;
	ViewPortDesc.Width = (_float)iWinSizeX;
	ViewPortDesc.Height = (_float)iWinSizeY;
	ViewPortDesc.MinDepth = 0.f;
	ViewPortDesc.MaxDepth = 1.f;

    m_iWinW = iWinSizeX;
    m_iWinH = iWinSizeY;

	m_pDeviceContext->RSSetViewports(1, &ViewPortDesc);

	*ppDevice = m_pDevice;
	*ppContext = m_pDeviceContext;

	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pDeviceContext);
	return S_OK;
}

void CGraphic_Device::On_Resize(_uint iWidth, _uint iHeight)
{
    if (iWidth == 0 || iHeight == 0)
        return;

    if (nullptr == m_pSwapChain || nullptr == m_pDeviceContext)
        return;

    /* 바인딩 해제 */
    m_pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    /* 기존 뷰 릴리즈 */
    Safe_Release(m_pDefaultRTV);
    Safe_Release(m_pDefaultDSV);

    /* 스왑체인 버퍼 리사이즈 */
    IF_FAIL_RETURN_MSG_BREAK(m_pSwapChain->ResizeBuffers(0, iWidth, iHeight, DXGI_FORMAT_UNKNOWN, 0), ,
        "ResizeBuffer failed.");

    /* 새 백버퍼로 RTV 재생성 + 새 DSV 생성 */
    IF_FAIL_RETURN_MSG_BREAK(Ready_Default_RTV(), ,
        "Ready DeafaultRTV failed.");

    if (FAILED(Ready_Default_DSV(iWidth, iHeight)))
        return;

    /* 상태 갱신 + 바인드 + 뷰포트 */
    m_iWinW = iWidth;
    m_iWinH = iHeight;

    Bind_DefaultRTV();
}

HRESULT CGraphic_Device::Ready_SceneRenderTarget(_uint iWidth, _uint iHeight)
{
    if (nullptr == m_pDevice)
        return E_FAIL;

    HRESULT hr = S_OK;

    ID3D11Texture2D* pSceneTexture = nullptr;
    ID3D11RenderTargetView* pSceneRTV = nullptr;
    ID3D11ShaderResourceView* pSceneSRV = nullptr;

    ID3D11Texture2D* pDepthStencilTexture = nullptr;
    ID3D11DepthStencilView* pSceneDSV = nullptr;
    ID3D11ShaderResourceView* pSceneDepthSRV = nullptr;

    /* --- Create Scene Texture --- */
    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = iWidth;
    textureDesc.Height = iHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    hr = m_pDevice->CreateTexture2D(&textureDesc, nullptr, &pSceneTexture);
    if (FAILED(hr))
    {
        Safe_Release(pSceneTexture);
        return E_FAIL;
    }

    hr = m_pDevice->CreateRenderTargetView(pSceneTexture, nullptr, &pSceneRTV);
    if (FAILED(hr))
    {
        Safe_Release(pSceneRTV);
        Safe_Release(pSceneTexture);
        return E_FAIL;
    }

    hr = m_pDevice->CreateShaderResourceView(pSceneTexture, nullptr, &pSceneSRV);
    if (FAILED(hr))
    {
        Safe_Release(pSceneSRV);
        Safe_Release(pSceneRTV);
        Safe_Release(pSceneTexture);
        return E_FAIL;
    }

    /* --- Create Scene Depth Stencil --- */
    D3D11_TEXTURE2D_DESC dsDesc{};
    dsDesc.Width = iWidth;
    dsDesc.Height = iHeight;
    dsDesc.MipLevels = 1;
    dsDesc.ArraySize = 1;
    dsDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    dsDesc.SampleDesc.Count = 1;
    dsDesc.SampleDesc.Quality = 0;
    dsDesc.Usage = D3D11_USAGE_DEFAULT;
    dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    dsDesc.CPUAccessFlags = 0;
    dsDesc.MiscFlags = 0;

    hr = m_pDevice->CreateTexture2D(&dsDesc, nullptr, &pDepthStencilTexture);
    if (FAILED(hr))
    {
        Safe_Release(pDepthStencilTexture);

        Safe_Release(pSceneSRV);
        Safe_Release(pSceneRTV);
        Safe_Release(pSceneTexture);

        return E_FAIL;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = m_pDevice->CreateDepthStencilView(pDepthStencilTexture, &dsvDesc, &pSceneDSV);
    if (FAILED(hr))
    {
        Safe_Release(pSceneDSV);
        Safe_Release(pDepthStencilTexture);

        Safe_Release(pSceneSRV);
        Safe_Release(pSceneRTV);
        Safe_Release(pSceneTexture);

        return E_FAIL;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = m_pDevice->CreateShaderResourceView(pDepthStencilTexture, &srvDesc, &pSceneDepthSRV);
    if (FAILED(hr))
    {
        Safe_Release(pSceneDepthSRV);
        Safe_Release(pSceneDSV);
        Safe_Release(pDepthStencilTexture);

        Safe_Release(pSceneSRV);
        Safe_Release(pSceneRTV);
        Safe_Release(pSceneTexture);

        return E_FAIL;
    }

    /* --- Commit : 성공 시에만 멤버에 붙인다. --- */
    Safe_Release(m_pSceneDSV);
    Safe_Release(m_pSceneSRV);
    Safe_Release(m_pSceneRTV);
    Safe_Release(m_pSceneTexture);
    Safe_Release(m_pSceneDepthSRV);

    m_pSceneTexture = pSceneTexture;         pSceneTexture = nullptr;
    m_pSceneRTV = pSceneRTV;                 pSceneRTV = nullptr;
    m_pSceneSRV = pSceneSRV;                 pSceneSRV = nullptr;
    m_pSceneDSV = pSceneDSV;                 pSceneDSV = nullptr;
    m_pSceneDepthSRV = pSceneDepthSRV;       pSceneDepthSRV = nullptr;

    Safe_Release(pDepthStencilTexture);

    return S_OK;
}

HRESULT CGraphic_Device::Clear_Default_RTV(const _float4* pClearColor)
{
	if (nullptr == m_pDeviceContext)
		return E_FAIL;

	m_pDeviceContext->ClearRenderTargetView(m_pDefaultRTV, reinterpret_cast<const _float*>(pClearColor));

	return S_OK;
}

HRESULT CGraphic_Device::Clear_Default_DSV()
{
	if (nullptr == m_pDeviceContext)
		return E_FAIL;

	m_pDeviceContext->ClearDepthStencilView(m_pDefaultDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	return S_OK;
}

HRESULT CGraphic_Device::Clear_Scene_RTV(const _float4* pClearColor)
{
	if (nullptr == m_pDeviceContext || nullptr == m_pSceneRTV)
		return E_FAIL;

	m_pDeviceContext->ClearRenderTargetView(m_pSceneRTV, reinterpret_cast<const _float*>(pClearColor));

	return S_OK;
}

HRESULT CGraphic_Device::Clear_Scene_DSV()
{
	if (nullptr == m_pDeviceContext || nullptr == m_pSceneDSV)
		return E_FAIL;

	m_pDeviceContext->ClearDepthStencilView(m_pSceneDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	return S_OK;
}

HRESULT CGraphic_Device::Present()
{
	if (nullptr == m_pSwapChain)
		return E_FAIL;

	return m_pSwapChain->Present(0, 0);
}

void CGraphic_Device::Bind_SceneRTV()
{
    m_pDeviceContext->OMSetRenderTargets(1, &m_pSceneRTV, m_pSceneDSV);
    Set_Viewport(m_iSceneW, m_iSceneH);
}

void CGraphic_Device::Bind_SceneRTV_WithoutDSV()
{
    m_pDeviceContext->OMSetRenderTargets(1, &m_pSceneRTV, nullptr);
    Set_Viewport(m_iDeferredW, m_iDeferredH);
}

HRESULT CGraphic_Device::Ensure_SceneRenderTarget(_uint w, _uint h)
{
    if (w == 0 || h == 0)
        return E_FAIL;

    /* 고정 해상도가 이미 정해져 있으면, 그 값 외에는 절대 재생성하지 않는다. */
    if (m_iFixedSceneW != 0 && m_iFixedSceneH != 0)
    {
        if (w != m_iFixedSceneW || h != m_iFixedSceneH)
            return S_OK; /* 외부 실수 호출 무시 */
    }
    else
    {
        /* 아직 고정값이 없다면 최초 1회만 고정값을 세팅한다. */
        m_iFixedSceneW = w;
        m_iFixedSceneH = h;
    }

    /* 이미 같은 고정 SceneRT가 있으면 종료 */
    if (m_iSceneW == m_iFixedSceneW && m_iSceneH == m_iFixedSceneH && m_pSceneSRV)
        return S_OK;

    Safe_Release(m_pSceneDSV);
    Safe_Release(m_pSceneSRV);
    Safe_Release(m_pSceneRTV);
    Safe_Release(m_pSceneTexture);

    m_iSceneW = m_iFixedSceneW;
    m_iSceneH = m_iFixedSceneH;

    return Ready_SceneRenderTarget(m_iSceneW, m_iSceneH);
}

void CGraphic_Device::Set_Viewport(_uint w, _uint h)
{
    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = (_float)w;
    vp.Height = (_float)h;
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;
    m_pDeviceContext->RSSetViewports(1, &vp);
}

void CGraphic_Device::Bind_DefaultRTV()
{
    m_pDeviceContext->OMSetRenderTargets(1, &m_pDefaultRTV, m_pDefaultDSV);
    Set_Viewport(m_iWinW, m_iWinH);
}

HRESULT CGraphic_Device::Ready_SwapChain(HWND hWnd, WINMODE isWindowed, _uint iWinCX, _uint iWinCY)
{
	IDXGIDevice* pDevice = nullptr;
	m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDevice);

	IDXGIAdapter* pAdapter = nullptr;
	pDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pAdapter);

	IDXGIFactory* pFactory = nullptr;
	pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pFactory);

	DXGI_SWAP_CHAIN_DESC		SwapChain;
	ZeroMemory(&SwapChain, sizeof(DXGI_SWAP_CHAIN_DESC));

	SwapChain.BufferDesc.Width = iWinCX;	
	SwapChain.BufferDesc.Height = iWinCY;	

	SwapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
	SwapChain.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChain.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	SwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChain.BufferCount = 1;

	SwapChain.BufferDesc.RefreshRate.Numerator = 60;
	SwapChain.BufferDesc.RefreshRate.Denominator = 1;

	SwapChain.SampleDesc.Quality = 0;
	SwapChain.SampleDesc.Count = 1;

	SwapChain.OutputWindow = hWnd;
	SwapChain.Windowed = static_cast<BOOL>(isWindowed);
	SwapChain.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (FAILED(pFactory->CreateSwapChain(m_pDevice, &SwapChain, &m_pSwapChain)))
		return E_FAIL;

	Safe_Release(pFactory);
	Safe_Release(pAdapter);
	Safe_Release(pDevice);

	return S_OK;
}


HRESULT CGraphic_Device::Ready_Default_RTV()
{
    if (nullptr == m_pDevice)
        return E_FAIL;

    ID3D11Texture2D* pBackBufferTexture = nullptr;

    if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture)))
        return E_FAIL;

    HRESULT hr = m_pDevice->CreateRenderTargetView(pBackBufferTexture, nullptr, &m_pDefaultRTV);
    Safe_Release(pBackBufferTexture); /* 실패 시에도 해제하도록 한다. */

    if (FAILED(hr))
        return E_FAIL;

    return S_OK;
}

HRESULT CGraphic_Device::Ready_Default_DSV(_uint iWinCX, _uint iWinCY)
{
	if (nullptr == m_pDevice)
		return E_FAIL;

	ID3D11Texture2D* pDepthStencilTexture = { nullptr };

	D3D11_TEXTURE2D_DESC	TextureDesc{};

	TextureDesc.Width = iWinCX;
	TextureDesc.Height = iWinCY;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT /* static */;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL /*| D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE*/;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pDepthStencilTexture)))
		return E_FAIL;

	if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pDefaultDSV)))
		return E_FAIL;

	Safe_Release(pDepthStencilTexture);

	return S_OK;
}

HRESULT CGraphic_Device::Ready_DeferredRenderTargets(_uint iWidth, _uint iHeight)
{
    if (nullptr == m_pDevice)
        return E_FAIL;

    ID3D11Texture2D* pDiffuseTexture = nullptr;
    ID3D11RenderTargetView* pDiffuseRTV = nullptr;
    ID3D11ShaderResourceView* pDiffuseSRV = nullptr;

    ID3D11Texture2D* pNormalTexture = nullptr;
    ID3D11RenderTargetView* pNormalRTV = nullptr;
    ID3D11ShaderResourceView* pNormalSRV = nullptr;

    ID3D11Texture2D* pLightTexture = nullptr;
    ID3D11RenderTargetView* pLightRTV = nullptr;
    ID3D11ShaderResourceView* pLightSRV = nullptr;

    if (FAILED(Create_RT_Texture(m_pDevice, iWidth, iHeight, DXGI_FORMAT_R8G8B8A8_UNORM,
        &pDiffuseTexture, &pDiffuseRTV, &pDiffuseSRV)))
        return E_FAIL;

    if (FAILED(Create_RT_Texture(m_pDevice, iWidth, iHeight, DXGI_FORMAT_R16G16B16A16_UNORM,
        &pNormalTexture, &pNormalRTV, &pNormalSRV)))
    {
        Safe_Release(pDiffuseSRV);
        Safe_Release(pDiffuseRTV);
        Safe_Release(pDiffuseTexture);
        return E_FAIL;
    }

    if (FAILED(Create_RT_Texture(m_pDevice, iWidth, iHeight, DXGI_FORMAT_R16G16B16A16_UNORM,
        &pLightTexture, &pLightRTV, &pLightSRV)))
    {
        Safe_Release(pNormalSRV);
        Safe_Release(pNormalRTV);
        Safe_Release(pNormalTexture);

        Safe_Release(pDiffuseSRV);
        Safe_Release(pDiffuseRTV);
        Safe_Release(pDiffuseTexture);
        return E_FAIL;
    }

    Safe_Release(m_pDiffuseSRV);
    Safe_Release(m_pDiffuseRTV);
    Safe_Release(m_pDiffuseTexture);

    Safe_Release(m_pNormalSRV);
    Safe_Release(m_pNormalRTV);
    Safe_Release(m_pNormalTexture);

    Safe_Release(m_pLightSRV);
    Safe_Release(m_pLightRTV);
    Safe_Release(m_pLightTexture);

    m_pDiffuseTexture = pDiffuseTexture;
    m_pDiffuseRTV = pDiffuseRTV;
    m_pDiffuseSRV = pDiffuseSRV;

    m_pNormalTexture = pNormalTexture;
    m_pNormalRTV = pNormalRTV;
    m_pNormalSRV = pNormalSRV;

    m_pLightTexture = pLightTexture;
    m_pLightRTV = pLightRTV;
    m_pLightSRV = pLightSRV;

    m_iDeferredW = iWidth;
    m_iDeferredH = iHeight;

    return S_OK;
}

HRESULT CGraphic_Device::Ensure_DeferredRenderTargets(_uint iWidth, _uint iHeight)
{
    if (iWidth == 0 || iHeight == 0)
        return E_FAIL;

    if (m_iDeferredW == iWidth && m_iDeferredH == iHeight &&
        m_pDiffuseSRV && m_pNormalSRV && m_pLightSRV)
        return S_OK;

    return Ready_DeferredRenderTargets(iWidth, iHeight);
}

void CGraphic_Device::Bind_GBufferRTV()
{
    ID3D11RenderTargetView* pRTVs[2] = { m_pDiffuseRTV, m_pNormalRTV };
    m_pDeviceContext->OMSetRenderTargets(2, pRTVs, m_pSceneDSV);
    Set_Viewport(m_iDeferredW, m_iDeferredH);
}

void CGraphic_Device::Bind_LightRTV()
{
    m_pDeviceContext->OMSetRenderTargets(1, &m_pLightRTV, m_pSceneDSV);
    Set_Viewport(m_iDeferredW, m_iDeferredH);
}

void CGraphic_Device::Bind_SceneSRV(_uint iSlot)
{
    m_pDeviceContext->PSSetShaderResources(iSlot, 1, &m_pSceneSRV);
}

void CGraphic_Device::Bind_SceneDepthSRV(_uint iSlot)
{
    m_pDeviceContext->PSSetShaderResources(iSlot, 1, &m_pSceneDepthSRV);
}

void CGraphic_Device::Unbind_PS_SRV(_uint iSlot)
{
    ID3D11ShaderResourceView* pNullSRV = nullptr;
    m_pDeviceContext->PSSetShaderResources(iSlot, 1, &pNullSRV);
}

HRESULT CGraphic_Device::Create_RT_Texture(
        ID3D11Device* pDevice,
        _uint iWidth,
        _uint iHeight,
        DXGI_FORMAT eFormat,
        ID3D11Texture2D** ppTexture,
        ID3D11RenderTargetView** ppRTV,
        ID3D11ShaderResourceView** ppSRV)
{
    if (nullptr == pDevice || nullptr == ppTexture || nullptr == ppRTV || nullptr == ppSRV)
        return E_FAIL;

    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = iWidth;
    textureDesc.Height = iHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = eFormat;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    if (FAILED(pDevice->CreateTexture2D(&textureDesc, nullptr, ppTexture)))
        return E_FAIL;

    if (FAILED(pDevice->CreateRenderTargetView(*ppTexture, nullptr, ppRTV)))
    {
        Safe_Release(*ppTexture);
        return E_FAIL;
    }

    if (FAILED(pDevice->CreateShaderResourceView(*ppTexture, nullptr, ppSRV)))
    {
        Safe_Release(*ppRTV);
        Safe_Release(*ppTexture);
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CGraphic_Device::Clear_Diffuse_RTV(const _float4* pClearColor)
{
    if (nullptr == m_pDeviceContext || nullptr == m_pDiffuseRTV)
        return E_FAIL;

    m_pDeviceContext->ClearRenderTargetView(m_pDiffuseRTV, reinterpret_cast<const _float*>(pClearColor));
    return S_OK;
}

HRESULT CGraphic_Device::Clear_Normal_RTV(const _float4* pClearColor)
{
    if (nullptr == m_pDeviceContext || nullptr == m_pNormalRTV)
        return E_FAIL;

    m_pDeviceContext->ClearRenderTargetView(m_pNormalRTV, reinterpret_cast<const _float*>(pClearColor));
    return S_OK;
}

HRESULT CGraphic_Device::Clear_Light_RTV(const _float4* pClearColor)
{
    if (nullptr == m_pDeviceContext || nullptr == m_pLightRTV)
        return E_FAIL;

    m_pDeviceContext->ClearRenderTargetView(m_pLightRTV, reinterpret_cast<const _float*>(pClearColor));
    return S_OK;
}

std::unique_ptr<CGraphic_Device> CGraphic_Device::Create(HWND hWnd, WINMODE isWindowed, _uint iWinSizeX, _uint iWinSizeY, ID3D11Device** ppDevice, ID3D11DeviceContext** ppDeviceContextOut)
{
    auto pInstance = std::make_unique<CGraphic_Device>();

	if (FAILED(pInstance->Initialize(hWnd, isWindowed, iWinSizeX, iWinSizeY, ppDevice, ppDeviceContextOut)))
	{
        MSG_BOX("Failed to Created : CGraphic_Device");
        return nullptr;
	}

	return pInstance;
}

NS_END
