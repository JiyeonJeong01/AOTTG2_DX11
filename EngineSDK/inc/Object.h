#pragma once
#include "Base.h"
class CObject : public CBase
{

};

template<typename T>
static T* Instantiate(LAYER_TYPE eLayer)
{
	T* pObject = new T();
	pObject->Set_LayerType(eLayer);

	return pObject;
}