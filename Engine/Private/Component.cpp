#include "Component.h"

CComponent::CComponent(COMPONENT_TYPE eComType)
	: m_eComType(eComType)
{

}

HRESULT CComponent::Init()
{
	return S_OK;
}

void CComponent::Render(const _float4x4& matView, const _float4x4& matProj)
{
}

void CComponent::Free()
{
	CBase::Free();
}
