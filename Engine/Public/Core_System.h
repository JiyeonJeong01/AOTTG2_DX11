#pragma once

#include "Engine_Define.h"
#include "Identity.h"

NS_BEGIN(Engine)

class CGraphic_Device;
class CTimer_Handler;
class CScene_Handler;
class CScene;

/**
 * \brief 엔진 전역 기반 인프라를 소유한다.
 */
class ENGINE_DLL CCore_System final
{
	DECLARE_SINGLETON(CCore_System)

public :
	HRESULT		Initialize_Engine(const ENGINE_DESC& EngineDesc, _Out_ ID3D11Device** ppDevice, _Out_ ID3D11DeviceContext** ppContext);
	void		Update_Editor_Engine(_float fDT);
    void        Update_Game_Engine(_float fDT);
    void        Fixed_Update(_float fDT);
    void        Request_Step(_float fDT, CScene* pScene);
    HRESULT     Draw();
    void        Clear_Resources(_uint iLevelIndex);

    void        On_Resize(EVENT_DATA& eData);

    void        Share_GraphicDevice(_Out_ ID3D11Device** ppDevice = nullptr, _Out_ ID3D11DeviceContext** ppContext = nullptr);
    void        Share_SceneSRV(_Out_ ID3D11ShaderResourceView** ppSRV);
    void        Share_DiffuseSRV(ID3D11ShaderResourceView** ppSRV);
    void        Share_NormalSRV(ID3D11ShaderResourceView** ppSRV);
    void        Share_LightSRV(ID3D11ShaderResourceView** ppSRV);
    void        Share_DepthSRV(ID3D11ShaderResourceView** ppSRV);
    void        Share_SpecularSRV(ID3D11ShaderResourceView** ppSRV);
    void        Share_PostProcessSRV(ID3D11ShaderResourceView** ppSRV);

    HRESULT     Ready_SceneRenderTarget(_uint iWidth, _uint iHeight);
    HRESULT     Ready_DeferredRenderTargets(_uint iWidth, _uint iHeight);

    void        Bind_DefaultRTV();
    void        Bind_SceneRTV();
    void        Bind_SceneRTV_WithoutDSV();
    void        Bind_GBufferRTV();
    void        Bind_LightRTV();
    void        Bind_PostProcessRTV();

    void        Bind_SceneSRV(_uint iSlot);
    void        Unbind_PS_SRV(_uint iSlot);

public :
	HRESULT		Clear_Default_Buffers(const _float4* pClearColor) const;
    HRESULT     Clear_Scene_Buffers(const _float4* pClearColor) const;
    HRESULT     Clear_GBuffer_Buffers(const _float4* pDiffuseClearColor, const _float4* pNormalClearColor) const;
    HRESULT     Clear_Light_Buffer(const _float4* pClearColor) const;
    HRESULT     Clear_Depth_RTV(const _float4* pClearColor);
    HRESULT     Clear_Specular_RTV(const _float4* pClearColor);
    HRESULT     Clear_PostProcess_RTV(const _float4* pClearColor);
	HRESULT		Present() const;


public: /* For.Timer_Manager */
	_float Compute_SystemDT() const;
	_float Compute_FrameDT() const;
    _float Get_FrameDT() const;

public: /* For.Scene_Handler */
    HRESULT Register_Scenes(const ASSET_GUID& tGUID, const std::filesystem::path& scenePath);
    HRESULT Open_EditScene(const std::string& sceneName);
    HRESULT Open_EditScene(const ASSET_GUID& tGUID);
    HRESULT Change_Scene(const std::string& sceneName);
    HRESULT Change_Scene(const ASSET_GUID& tGUID);
    HRESULT Save_CurrentScene(const std::filesystem::path& path);
    CScene* Get_CurrentScene();
    void    Set_CurrentScene(std::unique_ptr<CScene> pScene);
    _bool   Restart();

public : /* Debug */
    void    Set_DebugRender(DEBUG_DRAW eDraw);

private:
	std::unique_ptr<CGraphic_Device>		m_pGraphic_Device{ };
    std::unique_ptr<CTimer_Handler>		    m_pTimer_Handler{ };
    std::unique_ptr<CScene_Handler>         m_pScene_Handler{ };

    ID3D11Device*               m_pDevice{};
    ID3D11DeviceContext*        m_pContext{};
    ID3D11ShaderResourceView*   m_pSceneSRV{};

    _float                      m_fTimeAcc = 0.f;
    const _float                m_FIXED_DT = 0.02f;

private :
    void        Update_RuntimeEngine(_float fDT, CScene* pScene);

public :
    HRESULT Ready_ShadowRenderTargets(_uint iWidth, _uint iHeight);

    void Bind_StaticShadowRTV();
    void Bind_DynamicShadowRTV();

    HRESULT Clear_StaticLightDepth_RTV(const _float4* pClearColor);
    HRESULT Clear_DynamicLightDepth_RTV(const _float4* pClearColor);
    HRESULT Clear_Shadow_DSV();

    void Share_StaticLightDepthSRV(ID3D11ShaderResourceView** ppSRV);
    void Share_DynamicLightDepthSRV(ID3D11ShaderResourceView** ppSRV);

};
NS_END
