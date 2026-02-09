#pragma once
#include "Engine_Define.h"
#include "Logger.h"

NS_BEGIN(Engine)

class ENGINE_DLL ISink
{
public :
	virtual ~ISink() = default;
public :
	virtual void Write(const CLogger::RECORD& tRecord) = 0;
};

NS_END
