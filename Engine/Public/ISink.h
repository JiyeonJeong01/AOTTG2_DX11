#pragma once
#include "Base.h"
#include "Logger.h"

NS_BEGIN(Engine)

class ISink : public CBase 
{
protected :
	virtual ~ISink() = default;
public :
	virtual void Write(const CLogger::RECORD& tRecord) = 0;
};

NS_END
