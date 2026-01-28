#include "Logger.h"
#include "ConsoleSink.h"

IMPLEMENT_SINGLETON(CLogger)

void CLogger::Ready_Logger()
{
	/* TODO : ifdef 등 매크로 정의에 따라 바뀌긴 해야 한다. */
	m_pSink = new CConsoleSink();
}

void CLogger::Set_Sink(ISink* pSink)
{
	Safe_Release(m_pSink);
	m_pSink = pSink;
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
 * \brief printf 스타일 가변 인자를 std::string으로 포맷팅한다.
 * \param szFmt  printf 포맷 문자열
 * \param args printf 스타일의 가변 인자
 * \return va_list로 전달된 가변 인자를 vsnprintf로 포맷팅하여 string으로 반환한다
 */
string CLogger::FormatV(const char* szFmt, va_list args)
{
	if (szFmt == nullptr)
		return {};

	// va_list는 한 번 읽으면 내부 포인터가 이동하므로 복사 필수 
	va_list argsCopy;
	va_copy(argsCopy, args);
	const int len = vsnprintf(nullptr, 0, szFmt, argsCopy);
	va_end(argsCopy);

	if (len <= 0)
		return string(szFmt);

	string out;
	out.resize(SCAST(size_t, len) + 1);

	// szFmt + args를 해석하여 결과를 out 버퍼에 씀
	vsnprintf(&out[0], out.size(), szFmt, args);
	out.pop_back();
	return out;
}
