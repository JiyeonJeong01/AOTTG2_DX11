// AUTO-GENERATED. DO NOT EDIT.
#include "Script_Registry.h"
#include "Asset_Registry.h"
#include "Script_Handler.h"
#include "Script_Register.h"

#include "CScript_Test.h"

NS_BEGIN(Client)
void Register_AllScripts()
{
    auto& handler = SYS_ASSET.Scripts();
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\CScript_Test.script");
        handler.Register_VTable(guid, ScriptBinder<CScript_Test>::Build());
    }
}
NS_END