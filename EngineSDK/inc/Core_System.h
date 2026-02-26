#pragma once

#include "Engine_Define.h"
#include "Identity.h"

NS_BEGIN(Engine)

class CGraphic_Device;
class CTimer_Handler;
class CScene_Handler;
class CScene;

class ENGINE_DLL CCore_System final
{
	DECLARE_SINGLETON(CCore_System)

public :
	HRESULT		Initialize_Engine(const ENGINE_DESC& EngineDesc, _Out_ ID3D11Device** ppDevice, _Out_ ID3D11DeviceContext** ppContext);
	void		Update_Engine(_float fDT);
    void        Request_Step(_float fDT, CScene* pScene);
    HRESULT     Draw();
    void        Clear_Resources(_uint iLevelIndex);

    void        On_Resize(EVENT_DATA& eData);

    void        Share_GraphicDevice(_Out_ ID3D11Device** ppDevice = nullptr, _Out_ ID3D11DeviceContext** ppContext = nullptr);
    void        Share_SceneSRV(_Out_ ID3D11ShaderResourceView** ppSRV);
    HRESULT     Ready_SceneRenderTarget(_uint iWidth, _uint iHeight);
    void        Bind_DefaultRTV();
    void        Bind_SceneRTV();

public :
	HRESULT		Clear_Default_Buffers(const _float4* pClearColor) const;
    HRESULT     Clear_Scene_Buffers(const _float4* pClearColor) const;
	HRESULT		Present() const;

public: /* For.Timer_Manager */
	_float Compute_SystemDT() const;
	_float Compute_FrameDT() const;
    _float Get_FrameDT() const;

public: /* For.Scene_Handler */
    HRESULT Change_Scene(const ASSET_GUID& tGUID, SCENE_CHANGE_MODE eMode);
    HRESULT Save_CurrentScene(const std::filesystem::path& path);
    CScene* Get_CurrentScene();
    void    Set_CurrentScene(std::unique_ptr<CScene> pScene);

private:
	std::unique_ptr<CGraphic_Device>		m_pGraphic_Device{ };
    std::unique_ptr<CTimer_Handler>		    m_pTimer_Handler{ };
    std::unique_ptr<CScene_Handler>         m_pScene_Handler{ };

    ID3D11Device*               m_pDevice{};
    ID3D11DeviceContext*        m_pContext{};
    ID3D11ShaderResourceView*   m_pSceneSRV{};

private :
    void        Update_RuntimeEngine(_float fDT, CScene* pScene);

};
NS_END
