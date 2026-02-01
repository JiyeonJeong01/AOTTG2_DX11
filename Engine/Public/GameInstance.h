#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance)
private:
	CGameInstance();
	virtual ~CGameInstance() = default;

public :
	HRESULT		Initialize_Engine(const ENGINE_DESC& EngineDesc, _Out_ ID3D11Device** ppDevice, _Out_ ID3D11DeviceContext** ppContext);
	void		Update_Engine(_float fTimeDelta);
    HRESULT     Draw();
    void        Clear_Resources(_uint iLevelIndex);

    void        Share_GraphicDevice(_Out_ ID3D11Device** ppDevice, _Out_ ID3D11DeviceContext** ppContext);
public :
	HRESULT		Clear_Buffers(const _float4* pClearColor) const;
	HRESULT		Present() const;

public: /* For.Timer_Manager */
	_float Compute_SystemDT() const;
	_float Compute_FrameDT() const;

public: /* For.Scene_Manager */
    HRESULT Change_Scene(_uint iNewLevelIndex, class CLevel* pNewLevel);

//public: /* For.Prototype_Manager */
//    HRESULT Add_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag, CBase* pPrototype);
//    CBase* Clone_Prototype(PROTOTYPE ePrototype, _uint iLevelIndex, const _wstring& strPrototypeTag, void* pArg = nullptr);

private:
	class CGraphic_Device*		m_pGraphic_Device{ };
	class CTimer_System*		m_pTimerSystem{ };
    class CLevel_Manager*       m_pLevel_Manager { };
    class CPrototype_Manager*   m_pPrototype_Manager { };

    ID3D11Device*               m_pDevice{};
    ID3D11DeviceContext*        m_pContext{};

public:
	void Free() override;

};

NS_END
