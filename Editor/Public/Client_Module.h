#pragma once

#include "Editor_Define.h"

NS_BEGIN(Editor)

/* Client.dll(게임 Script)을 원본 대신 복사본으로 로드해, 에디터를 켠 채로 Client만 다시 빌드하고 바꿔 끼운다.
 * 원본을 직접 로드하면 실행 중에 파일이 잠겨 다시 빌드할 수 없다. */
namespace ClientModule
{
    inline constexpr const char* SCRIPT_REGISTRY_PATH = "../../Client/Private/Script_Registry.gen.cpp";

    _bool Load();     /* 복사본 로드 → Client_Register_AllScripts 호출 */
    _bool Reload();   /* Edit 상태에서만 : 씬 내리기 → DLL 교체 → 같은 씬 파일 다시 로드 */
    void  Unload();   /* 엔진이 Script를 모두 정리한 뒤에 호출 */
}

NS_END
