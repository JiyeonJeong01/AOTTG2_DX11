#include "GameObject.h"

Engine::CGameObject::CGameObject()
{
}



void Engine::CGameObject::Render()
{

}

COMPONENT_HANDLE Engine::CGameObject::Decode_Slot(uint32_t iSlotData) const
{
	COMPONENT_HANDLE h;
	h.iHandle = iSlotData & ComponentConfig::DATA_MASK;
	return h;
}

Engine::CGameObject* Engine::CGameObject::Create()
{
	CGameObject* pInstance = new CGameObject();


	return pInstance;
}

void Engine::CGameObject::Free()
{
	CBase::Free();
}
