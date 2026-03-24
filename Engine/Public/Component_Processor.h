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
	virtual HRESULT	Late_Initialize() { return S_OK; };
	virtual void	Update(_float fDT) { };
	virtual void	LateUpdate(_float fDT) { };

    virtual COMPONENT_HANDLE    Create_Component_Data(COMPONENT_TYPE eComType, OBJECT_HANDLE hObject) = 0;
    virtual void                Remove_Component(COMPONENT_TYPE eComType, COMPONENT_HANDLE hHandle) = 0;
    virtual HRESULT             Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec) = 0;
    virtual std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) = 0;
    virtual void                Set_Enable(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, _bool bEnable) = 0;
    virtual void*               Get_DataPtr(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) noexcept = 0;

    virtual                     PROCESSOR_ID Get_PID() const noexcept = 0;
protected :
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
};

#define DEF_PROCESSOR_ID(PID) \
public: \
    static constexpr PROCESSOR_ID Get_Static_PID() { return PID; } \
    virtual PROCESSOR_ID Get_PID() const noexcept override { return PID; }

NS_END
