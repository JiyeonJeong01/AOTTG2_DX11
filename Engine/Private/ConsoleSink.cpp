#include "ConsoleSink.h"

#include "Engine_Log.h"
#include "../../ThirdParty/magic_enum.hpp"

void CConsoleSink::Write(const CLogger::RECORD& tRecord)
{
    // 색 변경
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (tRecord.eSeverity) {
    case SEVERITY_TYPE::ERR:   SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY); break;
    case SEVERITY_TYPE::WARN:  SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); break;
    default:                   SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); break;
    }

    auto severity_view = magic_enum::enum_name(tRecord.eSeverity);

    printf("[%s][%s] %s (%s, Line: %d)\n",
        Get_TimeStamp().c_str(),
        severity_view.data(),                
        tRecord.strMsg.c_str(),
        tRecord.func,
        tRecord.iLine);
}

CConsoleSink* CConsoleSink::Create()
{
    return new CConsoleSink();
}

void CConsoleSink::Free()
{
	ISink::Free();
}
