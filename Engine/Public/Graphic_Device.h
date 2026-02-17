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

	HRESULT Clear_Default_RTV(const _float4* pClearColor);
	HRESULT Clear_Default_DSV();
    HRESULT Clear_Scene_RTV(const _float4* pClearColor);
    HRESULT Clear_Scene_DSV();
	HRESULT Present();

    HRESULT Ready_SceneRenderTarget(_uint iWidth, _uint iHeight);

    void    Bind_DefaultRTV();
    void    Bind_SceneRTV();
public:
    ID3D11ShaderResourceView* Get_SceneSRV() const { return m_pSceneSRV; }
    _uint Get_SceneW() const { return m_iSceneW; }
    _uint Get_SceneH() const { return m_iSceneH; }

    HRESULT Ensure_SceneRenderTarget(_uint w, _uint h); // 새로
    void    Set_Viewport(_uint w, _uint h);            // 새로

private:
	ID3D11Device*           m_pDevice { };
	ID3D11DeviceContext*    m_pDeviceContext { };
	IDXGISwapChain*         m_pSwapChain { };
    ID3D11RenderTargetView* m_pDefaultRTV{ };
    ID3D11DepthStencilView* m_pDefaultDSV{ };

    /* SceneView */
    ID3D11Texture2D*        m_pSceneTexture{ nullptr };
    ID3D11RenderTargetView* m_pSceneRTV{ nullptr };
    ID3D11ShaderResourceView* m_pSceneSRV{ nullptr };
    ID3D11DepthStencilView* m_pSceneDSV{ nullptr };

    _uint m_iSceneW = 0;
    _uint m_iSceneH = 0;
    _uint m_iWinW = 0;
    _uint m_iWinH = 0;

private:
	HRESULT Ready_SwapChain(HWND hWnd, WINMODE isWindowed, _uint iWinCX, _uint iWinCY);
	HRESULT Ready_Default_RTV();
	HRESULT Ready_Default_DSV(_uint iWinCX, _uint iWinCY);

public:
	static std::unique_ptr<CGraphic_Device> Create(_In_ HWND hWnd, WINMODE isWindowed, _uint iWinSizeX, _uint iWinSizeY, _Out_ ID3D11Device** ppDevice, _Out_ ID3D11DeviceContext** ppDeviceContextOut);
};

NS_END
