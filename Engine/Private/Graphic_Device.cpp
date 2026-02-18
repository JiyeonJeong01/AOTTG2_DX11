#include "..\public\Graphic_Device.h"

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
    /* TODO -------------------------------------------------------------*/
    /* TODO -------------------------------------------------------------*/
    /* TODO -------------------------------------------------------------*/
    /* TODO -------------------------------------------------------------*/
    /* TODO                             RESIZE 처리                      */
    /* TODO -------------------------------------------------------------*/
    /* TODO -------------------------------------------------------------*/
    /* TODO -------------------------------------------------------------*/
    /* TODO -------------------------------------------------------------*/
}

HRESULT CGraphic_Device::Ready_SceneRenderTarget(_uint iWidth, _uint iHeight)
{
    if (nullptr == m_pDevice) return E_FAIL;

    // --- Create Scene Texture & RTV & SRV ---
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

    if (FAILED(m_pDevice->CreateTexture2D(&textureDesc, nullptr, &m_pSceneTexture)))
        return E_FAIL;

    if (FAILED(m_pDevice->CreateRenderTargetView(m_pSceneTexture, nullptr, &m_pSceneRTV)))
        return E_FAIL;

    if (FAILED(m_pDevice->CreateShaderResourceView(m_pSceneTexture, nullptr, &m_pSceneSRV)))
        return E_FAIL;


    // --- Create Scene Depth Stencil View ---
    ID3D11Texture2D* pDepthStencilTexture = nullptr;
    D3D11_TEXTURE2D_DESC dsDesc{};
    dsDesc.Width = iWidth;
    dsDesc.Height = iHeight;
    dsDesc.MipLevels = 1;
    dsDesc.ArraySize = 1;
    dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsDesc.SampleDesc.Count = 1;
    dsDesc.SampleDesc.Quality = 0;
    dsDesc.Usage = D3D11_USAGE_DEFAULT;
    dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    if (FAILED(m_pDevice->CreateTexture2D(&dsDesc, nullptr, &pDepthStencilTexture)))
        return E_FAIL;

    if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pSceneDSV)))
        return E_FAIL;

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

HRESULT CGraphic_Device::Ensure_SceneRenderTarget(_uint w, _uint h)
{
    if (w == 0 || h == 0) return E_FAIL;
    if (m_iSceneW == w && m_iSceneH == h && m_pSceneSRV) return S_OK;

    Safe_Release(m_pSceneDSV);
    Safe_Release(m_pSceneSRV);
    Safe_Release(m_pSceneRTV);
    Safe_Release(m_pSceneTexture);

    m_iSceneW = w;
    m_iSceneH = h;

    return Ready_SceneRenderTarget(w, h);
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

	if (FAILED(m_pDevice->CreateRenderTargetView(pBackBufferTexture, nullptr, &m_pDefaultRTV)))
		return E_FAIL;

	Safe_Release(pBackBufferTexture);

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
