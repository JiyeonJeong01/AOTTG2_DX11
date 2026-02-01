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

		const _char*    szFile; // __FILE__
		const _char*    szFunc; // __FUNCTION__
		_int			iLine;

		const char*     szExpr;
		std::string		strMsg{};
        tagRecord()
        : eSeverity(SEVERITY_TYPE::END), eDomain(DOMAIN_TYPE::END), szFile(nullptr), szFunc(nullptr), iLine(0), szExpr(nullptr) {}
        tagRecord(SEVERITY_TYPE eSev, DOMAIN_TYPE eDom, const char* file, const char* func, _int line, const _char* expr)
        : eSeverity(eSev), eDomain(eDom), szFile(file), szFunc(func), iLine(line), szExpr(expr) {}
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
	static string	FormatV(const char* szFmt, ...);

private :
	ISink* m_pSink{};
    string Get_TimeStamp() const;

    virtual void	Free() {};
};

NS_END
