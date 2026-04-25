#pragma once

#include "Engine_Define.h"

NS_BEGIN(Engine)

class CGraphic_Device final
{
public:
	CGraphic_Device();
	~CGraphic_Device();

public:
	HRESULT Initialize(HWND hWnd, WINMODE isWindowed, _uint iWinSizeX, _uint iWinSizeY, _Inout_ ID3D11Device** ppDevice, _Inout_ ID3D11DeviceContext** ppContext);
    HRESULT Ready_SceneRenderTarget(_uint iWidth, _uint iHeight);

    void    On_Resize(_uint iWidth, _uint iHeight);

	HRESULT Clear_Default_RTV(const _float4* pClearColor);
	HRESULT Clear_Default_DSV();
    HRESULT Clear_Scene_RTV(const _float4* pClearColor);
    HRESULT Clear_Scene_DSV();

	HRESULT Present();

    void    Bind_DefaultRTV();
    void    Bind_SceneRTV();
    void    Bind_SceneRTV_WithoutDSV();
    void    Bind_GBufferRTV();
    void    Bind_LightRTV();
    void    Bind_PostProcessRTV();
    void    Bind_StaticShadowRTV();
    void    Bind_DynamicShadowRTV();

    void    Bind_SceneSRV(_uint iSlot);
    void    Unbind_PS_SRV(_uint iSlot);
public:
    ID3D11ShaderResourceView*   Get_SceneSRV() const { return m_pSceneSRV; }
    _uint                       Get_SceneW() const { return m_iSceneW; }
    _uint                       Get_SceneH() const { return m_iSceneH; }

    HRESULT Ensure_SceneRenderTarget(_uint w, _uint h); // 새로
    void    Set_Viewport(_uint w, _uint h);            // 새로
    HRESULT Ensure_ShadowRenderTargets(_uint iWidth, _uint iHeight);
    HRESULT Ready_ShadowRenderTargets(_uint iWidth, _uint iHeight);
public:
    HRESULT Ready_DeferredRenderTargets(_uint iWidth, _uint iHeight);
    HRESULT Ensure_DeferredRenderTargets(_uint iWidth, _uint iHeight);

    HRESULT Clear_Diffuse_RTV(const _float4* pClearColor);
    HRESULT Clear_Normal_RTV(const _float4* pClearColor);
    HRESULT Clear_Depth_RTV(const _float4* pClearColor);
    HRESULT Clear_Light_RTV(const _float4* pClearColor);
    HRESULT Clear_Specular_RTV(const _float4* pClearColor);
    HRESULT Clear_PostProcess_RTV(const _float4* pClearColor);


    HRESULT Clear_StaticLightDepth_RTV(const _float4* pClearColor);
    HRESULT Clear_DynamicLightDepth_RTV(const _float4* pClearColor);
    HRESULT Clear_Shadow_DSV();

public:
    ID3D11ShaderResourceView* Get_DiffuseSRV() const { return m_pDiffuseSRV; }
    ID3D11ShaderResourceView* Get_NormalSRV() const { return m_pNormalSRV; }
    ID3D11ShaderResourceView* Get_DepthSRV() const { return m_pDepthSRV; }
    ID3D11ShaderResourceView* Get_LightSRV() const { return m_pLightSRV; }
    ID3D11ShaderResourceView* Get_SpecularSRV() const { return m_pSpecularSRV; }
    ID3D11ShaderResourceView* Get_PostProcessSRV() const { return m_pPostProcessSRV; }
    ID3D11ShaderResourceView* Get_StaticLightDepthSRV() const { return m_pStaticLightDepthSRV; }
    ID3D11ShaderResourceView* Get_DynamicLightDepthSRV() const { return m_pDynamicLightDepthSRV; }

private:
	HRESULT Ready_SwapChain(HWND hWnd, WINMODE isWindowed, _uint iWinCX, _uint iWinCY);
	HRESULT Ready_Default_RTV();
	HRESULT Ready_Default_DSV(_uint iWinCX, _uint iWinCY);
    HRESULT Create_RT_Texture(
        ID3D11Device* pDevice,
        _uint iWidth,
        _uint iHeight,
        DXGI_FORMAT eFormat,
        ID3D11Texture2D** ppTexture,
        ID3D11RenderTargetView** ppRTV,
        ID3D11ShaderResourceView** ppSRV);

private:
    ID3D11Device* m_pDevice{ };
    ID3D11DeviceContext* m_pDeviceContext{ };
    IDXGISwapChain* m_pSwapChain{ };

    _uint                       m_iSceneW = 0;
    _uint                       m_iSceneH = 0;
    _uint                       m_iFixedSceneW = 0;
    _uint                       m_iFixedSceneH = 0;
    _uint                       m_iWinW = 0;
    _uint                       m_iWinH = 0;
    _uint                       m_iDeferredW = 0;
    _uint                       m_iDeferredH = 0;

    /* Back buffer */
    ID3D11RenderTargetView* m_pDefaultRTV{ };
    ID3D11DepthStencilView* m_pDefaultDSV{ };

    /* SceneView */
    ID3D11Texture2D* m_pSceneTexture{ nullptr };
    ID3D11RenderTargetView* m_pSceneRTV{ nullptr };
    ID3D11ShaderResourceView* m_pSceneSRV{ nullptr };
    ID3D11DepthStencilView* m_pSceneDSV{ nullptr };

private:
    /* Deferred */
    ID3D11Texture2D* m_pDiffuseTexture{ nullptr };
    ID3D11RenderTargetView* m_pDiffuseRTV{ nullptr };
    ID3D11ShaderResourceView* m_pDiffuseSRV{ nullptr };

    ID3D11Texture2D* m_pNormalTexture{ nullptr };
    ID3D11RenderTargetView* m_pNormalRTV{ nullptr };
    ID3D11ShaderResourceView* m_pNormalSRV{ nullptr };

    ID3D11Texture2D* m_pLightTexture{ nullptr };
    ID3D11RenderTargetView* m_pLightRTV{ nullptr };
    ID3D11ShaderResourceView* m_pLightSRV{ nullptr };

    ID3D11Texture2D* m_pDepthTexture = nullptr;
    ID3D11RenderTargetView* m_pDepthRTV = nullptr;
    ID3D11ShaderResourceView* m_pDepthSRV = nullptr;

    ID3D11Texture2D* m_pSpecularTexture = nullptr;
    ID3D11RenderTargetView* m_pSpecularRTV = nullptr;
    ID3D11ShaderResourceView* m_pSpecularSRV = nullptr;

    ID3D11Texture2D* m_pPostProcessTexture = nullptr;
    ID3D11RenderTargetView* m_pPostProcessRTV = nullptr;
    ID3D11ShaderResourceView* m_pPostProcessSRV = nullptr;

    ID3D11Texture2D* m_pStaticLightDepthTexture = nullptr;
    ID3D11RenderTargetView* m_pStaticLightDepthRTV = nullptr;
    ID3D11ShaderResourceView* m_pStaticLightDepthSRV = nullptr;

    ID3D11Texture2D* m_pDynamicLightDepthTexture = nullptr;
    ID3D11RenderTargetView* m_pDynamicLightDepthRTV = nullptr;
    ID3D11ShaderResourceView* m_pDynamicLightDepthSRV = nullptr;

    ID3D11Texture2D* m_pShadowDepthStencilTexture = nullptr;
    ID3D11DepthStencilView* m_pShadowDSV = nullptr;

    _uint                       m_iShadowW = 0;
    _uint                       m_iShadowH = 0;

public:
	static std::unique_ptr<CGraphic_Device> Create(_In_ HWND hWnd, WINMODE isWindowed, _uint iWinSizeX, _uint iWinSizeY, _Out_ ID3D11Device** ppDevice, _Out_ ID3D11DeviceContext** ppDeviceContextOut);
};

NS_END
