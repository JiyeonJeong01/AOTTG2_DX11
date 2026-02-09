#include "CLI_Sink.h"

#include "Engine_Log.h"
#include "magic_enum.hpp"

NS_BEGIN(Engine)

CCLI_Sink::CCLI_Sink()
{
}

CCLI_Sink::~CCLI_Sink()
{
}

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

std::unique_ptr<CCLI_Sink> CCLI_Sink::Create()
{
    return std::make_unique<CCLI_Sink>();
}

NS_END
