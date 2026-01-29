#pragma once

#include "Base.h"

NS_BEGIN(Engine)
class CComponent_Processor;
class CComponentGroup_Manager;

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
public :
	HRESULT		Clear_Buffers(const _float4* pClearColor) const;
	HRESULT		Present() const;

public :

public: /* For.Timer_Manager */
	_float Compute_SystemDT() const;
	_float Compute_FrameDT() const;

private:
	class CGraphic_Device*		m_pGraphic_Device{ };
	class CTimerSystem*			m_pTimerSystem{ };


public:
	void Free() override;

};

NS_END
