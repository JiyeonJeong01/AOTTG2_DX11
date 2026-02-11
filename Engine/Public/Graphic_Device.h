#pragma once

#include "Engine_Define.h"

NS_BEGIN(Engine)

class CGraphic_Device final
{
public:
	CGraphic_Device();
	~CGraphic_Device();

public:
	HRESULT Initialize(HWND hWnd, WINMODE isWindowed, _uint iWinSizeX, _uint iWinSizeY,
		                _Inout_ ID3D11Device** ppDevice, _Inout_ ID3D11DeviceContext** ppContext);

	HRESULT Clear_BackBuffer_View(const _float4* pClearColor);
	HRESULT Clear_DepthStencil_View();

	HRESULT Present();

    /* ------------------ TEST ------------------ */
    HRESULT Ready_SceneRenderTarget(_uint w, _uint h);
    void Bind_SceneRT();
    void Bind_BackBuffer();
    void Clear_SceneRTV(const _float4* pClearColor);
    void Clear_SceneDSV();
    
    ID3D11ShaderResourceView* Get_SceneSRV() const { return m_pSceneSRV; }

private:
	ID3D11Device*           m_pDevice { };
	ID3D11DeviceContext*    m_pDeviceContext { };
	IDXGISwapChain*         m_pSwapChain { };

	ID3D11RenderTargetView* m_pBackBufferRTV { };
	ID3D11DepthStencilView* m_pDepthStencilView { };


    /* 오프스크린 */
    ID3D11Texture2D*        m_pSceneTex = nullptr;
    ID3D11RenderTargetView* m_pSceneRTV = nullptr;
    ID3D11ShaderResourceView* m_pSceneSRV = nullptr;
    ID3D11Texture2D*        m_pSceneDepthTex = nullptr;
    ID3D11DepthStencilView* m_pSceneDSV = nullptr;

    _uint m_iSceneW = 0;
    _uint m_iSceneH = 0;

private:
	HRESULT Ready_SwapChain(HWND hWnd, WINMODE isWindowed, _uint iWinCX, _uint iWinCY);
	HRESULT Ready_BackBufferRenderTargetView();
	HRESULT Ready_DepthStencilView(_uint iWinCX, _uint iWinCY);

public:
	static std::unique_ptr<CGraphic_Device> Create(_In_ HWND hWnd, WINMODE isWindowed, _uint iWinSizeX, _uint iWinSizeY, _Out_ ID3D11Device** ppDevice, _Out_ ID3D11DeviceContext** ppDeviceContextOut);
};

NS_END
