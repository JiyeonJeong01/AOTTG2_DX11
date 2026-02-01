#pragma once
#include "ISink.h"

NS_BEGIN(Engine)

class CCLI_Sink final :  public ISink
{
private:
	~CCLI_Sink() = default;
public :
	void Write(const CLogger::RECORD& tRecord) override;

public :
	static CCLI_Sink* Create();
private:
	void Free() override;
};

NS_END
