#include "Logger.h"
#include "CLI_Sink.h"
#include "GUI_Sink.h"

NS_BEGIN(Engine)

IMPLEMENT_SINGLETON(CLogger)

HRESULT CLogger::Initialize()
{
    return S_OK;
}

ISink* CLogger::Set_Sink(LOG_TYPE eType)
{
	Safe_Release(m_pSink);

    switch (eType)
    {
    case LOG_TYPE::CLI:
        m_pSink = new CCLI_Sink;
        break;
    case LOG_TYPE::GUI:
        m_pSink = new CGUI_Sink;
        break;
    }

    return m_pSink;
}

void CLogger::Log(SEVERITY_TYPE eSeverity, DOMAIN_TYPE eDomain, const char* szFile, const char* szFunc,
	const char* szExpr, int iLine, const char* szFmt, ...)
{
	if (m_pSink == nullptr)
		return;

	if (szFmt == nullptr)
		szFmt = "";

	va_list args;
	va_start(args, szFmt);

	string msg = FormatV(szFmt, args);

	va_end(args);

	RECORD tRecord;
	tRecord.eSeverity = eSeverity;
	tRecord.eDomain = eDomain;
	tRecord.file = szFile;
	tRecord.func = szFunc;
	tRecord.iLine = iLine;
	tRecord.expr = szExpr;
	tRecord.strMsg = msg;

	m_pSink->Write(tRecord);
}

void CLogger::Assert(DOMAIN_TYPE eDomain, const char* szFile, const char* szFunc,
	const char* szExpr, int iLine, const char* szFmt, ...)
{
	if (m_pSink == nullptr)
		return;

	string msg;
	if (szFmt == nullptr || szFmt[0] == '\0')
	{
		msg = "Assertion Failed";
		if (szExpr && szExpr[0] != '\0')
		{
			msg += " : ";
			msg += szExpr;
		}
	}
	else
	{
		va_list args;
		va_start(args, szFmt);

		msg = FormatV(szFmt, args);

		va_end(args);
	}

	RECORD tRecord;
	tRecord.eSeverity = SEVERITY_TYPE::ASSERTION;
	tRecord.eDomain = eDomain;
	tRecord.file = szFile;
	tRecord.func = szFunc;
	tRecord.iLine = iLine;
	tRecord.expr = szExpr;
	tRecord.strMsg = msg;

	m_pSink->Write(tRecord);
}

/**
 * \brief Formats printf-style variadic arguments into std::string
 * \param szFmt  printf-style format string
 * \param args Variadic arguments in printf-style 
 * \return a std::string containing the result of formatting the variadic arguments using vsnprintf.
 */
string CLogger::FormatV(const char* szFmt, va_list args)
{
    if (!szFmt)
        return {};

    /* va_list must be copied because reading it moves its internal pointer. */
    va_list argsCopy;
    va_copy(argsCopy, args);
    const int len = std::vsnprintf(nullptr, 0, szFmt, argsCopy);
    va_end(argsCopy);

    if (len < 0)
        return {};

    std::string out;
    out.resize(static_cast<size_t>(len) + 1); // +1 for null terminator

    /* Parse szFmt and args, and write the formatted result into output buffer */
    std::vsnprintf(out.data(), out.size(), szFmt, args);

    out.pop_back(); /* remove null terminator */
    return out;
}

NS_END
