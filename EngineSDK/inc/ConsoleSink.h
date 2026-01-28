#pragma once
#include "ISink.h"

NS_BEGIN(Engine)

class CConsoleSink final :  public ISink
{
private:
	~CConsoleSink() = default;
public :
	void Write(const CLogger::RECORD& tRecord) override;

public :
	static CConsoleSink* Create();
private:
	void Free() override;
};

NS_END