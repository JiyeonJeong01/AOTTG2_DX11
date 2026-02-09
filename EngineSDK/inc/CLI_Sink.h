#pragma once
#include "ISink.h"

NS_BEGIN(Engine)

class CCLI_Sink final :  public ISink
{
public:
    CCLI_Sink();
    ~CCLI_Sink() override;
public:
	void Write(const CLogger::RECORD& tRecord) override;

public :
	static std::unique_ptr<CCLI_Sink> Create();
};

NS_END
