  #pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

class ENGINE_DLL CComponent_Processor abstract
{
public:
	CComponent_Processor() {};
    virtual ~CComponent_Processor() = default;

public :
	virtual HRESULT	Initialize() { return S_OK; };
	virtual void	Update(_float fDT) { };
	virtual void	LateUpdate(_float fDT) { };

	virtual COMPONENT_HANDLE Create_Component_Data() { return COMPONENT_HANDLE{}; }
	virtual void Remove_Component(COMPONENT_HANDLE hHandle) {};
    virtual HRESULT Initialize_From_Spec(COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec) { return S_OK; };

protected :
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
};

NS_END
