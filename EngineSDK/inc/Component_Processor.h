  #pragma once
#include "Engine_Define.h"
#include "Spec_Struct.h"
#include "Component_Struct.h"
#include "Object_Struct.h"

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

    virtual COMPONENT_HANDLE    Create_Component_Data(COMPONENT_TYPE eComType, OBJECT_HANDLE hObject) = 0;
    virtual void                Remove_Component(COMPONENT_TYPE eComType, COMPONENT_HANDLE hHandle) = 0;
    virtual HRESULT             Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec) = 0;
    virtual std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) = 0;

protected :
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
};

NS_END
