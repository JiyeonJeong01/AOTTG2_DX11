#pragma once

#include "Engine_Define.h"

NS_BEGIN(Engine)

class CGraphic_Device;
class CTimer_System;
class CScene_Manager;
class CPrototype_Manager;

class ENGINE_DLL CCore_System final
{
	DECLARE_SINGLETON(CCore_System)

public :
	HRESULT		Initialize_Engine(const ENGINE_DESC& EngineDesc, _Out_ ID3D11Device** ppDevice, _Out_ ID3D11DeviceContext** ppContext);
	void		Update_Engine(_float fTimeDelta);
    HRESULT     Draw();
    void        Clear_Resources(_uint iLevelIndex);

    void        Share_GraphicDevice(_Out_ ID3D11Device** ppDevice = nullptr,
                                    _Out_ ID3D11DeviceContext** ppContext = nullptr,
                                    _Out_ ID3D11ShaderResourceView** ppSceneSRV = nullptr);
public :
	HRESULT		Clear_Buffers(const _float4* pClearColor) const;
	HRESULT		Present() const;

public :
    HRESULT Ready_SceneRenderTarget(_uint w, _uint h);
    void Bind_SceneRT();
    void Bind_BackBuffer();
    void Clear_SceneRTV(const _float4* pClearColor);
    void Clear_SceneDSV();

public: /* For.Timer_Manager */
	_float Compute_SystemDT() const;
	_float Compute_FrameDT() const;

public: /* For.Scene_Manager */
    HRESULT Change_Scene(_uint iNewLevelIndex, class CLevel* pNewLevel);

//public: /* For.Prototype_Manager */
//    HRESULT Add_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag, CBase* pPrototype);
//    CBase* Clone_Prototype(PROTOTYPE ePrototype, _uint iLevelIndex, const _wstring& strPrototypeTag, void* pArg = nullptr);

private:
	std::unique_ptr<CGraphic_Device>		m_pGraphic_Device{ };
    std::unique_ptr<CTimer_System>		    m_pTimerSystem{ };

    ID3D11Device*               m_pDevice{};
    ID3D11DeviceContext*        m_pContext{};
    ID3D11ShaderResourceView*   m_pSceneSRV{};

};

NS_END
