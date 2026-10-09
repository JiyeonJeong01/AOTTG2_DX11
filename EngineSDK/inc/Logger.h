#pragma once
#include "Base.h"
#include <atomic>

NS_BEGIN(Engine)
    class ISink;
/**
 * CLogger is used across multiple modules (.dll, .exe, and .lib),
 * so it is managed as a singleton and exported via dllexport.
 */
class ENGINE_DLL CLogger : public CBase
{
	DECLARE_SINGLETON(CLogger)

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
    /* 타입별 sink를 Logger가 끝까지 소유한다. Set_Sink로 교체해도 이전 sink를 파괴하지 않으므로 반환한 포인터가 댕글링되지 않는다. */
    std::unique_ptr<ISink>  m_pSinks[static_cast<size_t>(LOG_TYPE::END)]{};
    /* 활성 sink. sink 없는 빌드(Game.exe)에서 포맷 전에 lock 없이 빠르게 반환하기 위해 atomic */
    std::atomic<ISink*>     m_pSink{ nullptr };
    /* Set_Sink 교체와 Write 호출을 직렬화 */
    std::mutex              m_mtx;

    void   Write(const RECORD& tRecord);
    string Get_TimeStamp() const;

    virtual void	Free() {};
};

NS_END
