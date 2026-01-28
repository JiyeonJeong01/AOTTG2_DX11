#pragma once
#include "Base.h"

NS_BEGIN(Engine)

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

public :
	void	Ready_Logger();
	void	Set_Sink(class ISink* pSink);

public :
	void	Log(SEVERITY_TYPE eSeverity, DOMAIN_TYPE eDomain, 
					const char* szFile, const char* szFunc, const char* szExpr, int iLine, const char* szFmt, ...);
	void	Assert(DOMAIN_TYPE eDomain,
					const char* szFile, const char* szFunc, const char* szExpr, int iLine, const char* szFmt, ...);
private :
	static string	FormatV(const char* szFmt, va_list args);
	ISink* m_pSink{};

	virtual void	Free() {};
};

NS_END