#include "CLI_Sink.h"

#include "Engine_Log.h"
#include "magic_enum.hpp"

NS_BEGIN(Engine)

void CCLI_Sink::Write(const CLogger::RECORD& tRecord)
{
    // Change Color
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (tRecord.eSeverity) {
    case SEVERITY_TYPE::ERR:   SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY); break;
    case SEVERITY_TYPE::WARN:  SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); break;
    default:                   SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); break;
    }

    printf("%s", tRecord.strMsg.c_str());
}

CCLI_Sink* CCLI_Sink::Create()
{
    return new CCLI_Sink();
}

void CCLI_Sink::Free()
{
	ISink::Free();
}

NS_END
