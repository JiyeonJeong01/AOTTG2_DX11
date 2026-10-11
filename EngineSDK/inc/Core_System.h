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
    void        Copy_Scene_To_BackBuffer(); /* Game.exe 전용 : 에디터는 SceneSRV를 ImGui로 띄운다 */


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
    void    Request_RestartScene();
    HRESULT Save_CurrentScene(const std::filesystem::path& path);
    CScene* Get_CurrentScene();
    void    Set_CurrentScene(std::unique_ptr<CScene> pScene);
    _bool   Restart();
    _bool   Unload_Scripts(); /* 핫 리로드 : 씬 오브젝트를 모두 내리고 Script 함수 주소를 버린다. 이후 Client.dll을 내려도 안전 */

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
    _bool                       m_bRestartSceneRequested = false;

    /* 물리 고정 시간 루프. 게임 갱신이 60Hz라 1/60이면 프레임당 1스텝
     * ponytail: 렌더 보간 없음. 갱신 주기가 60Hz가 아니면 보간(이전·현재 Transform 블렌드)을 추가할 것 */
    static constexpr _float     FIXED_STEP = 1.f / 60.f;
    /* 메인 루프가 이미 60Hz로 틱하므로, 틱 하나(±반 스텝)는 정확히 물리 1스텝으로 본다.
     * 2스텝 이상은 프레임이 실제로 밀렸을 때(1.5 스텝 이상)만 돈다 */
    static constexpr _float     FIXED_STEP_SNAP = FIXED_STEP * 0.5f;
    static constexpr _uint      MAX_FIXED_STEPS = 5;
    _float                      m_fFixedAcc = 0.f;

private :
    void        Update_RuntimeEngine(_float fDT, CScene* pScene);
    _float      Clamp_FrameDT(_float fDT) const;
    void        Process_RestartSceneRequest();

public :
    HRESULT Ready_ShadowRenderTargets(_uint iWidth, _uint iHeight);
    HRESULT Set_ShadowFormat(DXGI_FORMAT eFormat);
    DXGI_FORMAT Get_ShadowFormat() const;

    void Bind_StaticShadowRTV();
    void Bind_DynamicShadowRTV();

    HRESULT Clear_StaticLightDepth_RTV(const _float4* pClearColor);
    HRESULT Clear_DynamicLightDepth_RTV(const _float4* pClearColor);
    HRESULT Clear_Shadow_DSV();

    void Share_StaticLightDepthSRV(ID3D11ShaderResourceView** ppSRV);
    void Share_DynamicLightDepthSRV(ID3D11ShaderResourceView** ppSRV);

};
NS_END
