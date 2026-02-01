#pragma once
#include "Base.h"

NS_BEGIN(Engine)
    class ISink;
/**
 * CLogger is used across multiple modules (.dll, .exe, and .lib),
 * so it is managed as a singleton and exported via dllexport.
 */
class ENGINE_DLL CLogger : public CBase
{
	DECLARE_SINGLETON(CLogger)
private:
	CLogger() {};
	virtual ~CLogger() = default;
public :
	typedef struct tagRecord
	{
		SEVERITY_TYPE	eSeverity;
		DOMAIN_TYPE		eDomain;

		const char* file; // __FILE__
		const char* func; // __FUNCTION__
		_int			iLine;

		const char* expr;
		std::string			strMsg;
	}RECORD;

    enum class LOG_TYPE { CLI, GUI, END };

public :
    HRESULT	Initialize();
    ISink*	Set_Sink(LOG_TYPE eType);

	void	Log(SEVERITY_TYPE eSeverity, DOMAIN_TYPE eDomain, 
					const char* szFile, const char* szFunc, const char* szExpr, int iLine, const char* szFmt, ...);
	void	Assert(DOMAIN_TYPE eDomain,
					const char* szFile, const char* szFunc, const char* szExpr, int iLine, const char* szFmt, ...);
	static string	FormatV(const char* szFmt, va_list args);
	ISink* m_pSink{};

	virtual void	Free() {};
};

NS_END
