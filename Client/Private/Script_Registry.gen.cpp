// AUTO-GENERATED. DO NOT EDIT.
#include "Script_Registry.h"
#include "Asset_Registry.h"
#include "Script_Handler.h"
#include "Script_Register.h"

#include "CameraController.h"
#include "FirstScene.h"
#include "Hello.h"
#include "MainMenu_Controller.h"
#include "Player.h"
#include "PrototypeTest.h"
#include "CScript_Test.h"
#include "Test_ComponentEnable.h"

NS_BEGIN(Client)
void Register_AllScripts()
{
    auto& handler = SYS_ASSET.Scripts();
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\CameraController.script");
        handler.Register_VTable(guid, ScriptBinder<CCameraController>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\FirstScene.script");
        handler.Register_VTable(guid, ScriptBinder<CFirstScene>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\Hello.script");
        handler.Register_VTable(guid, ScriptBinder<CHello>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\MainMenu_Controller.script");
        handler.Register_VTable(guid, ScriptBinder<CMainMenu_Controller>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\Player.script");
        handler.Register_VTable(guid, ScriptBinder<CPlayer>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\PrototypeTest.script");
        handler.Register_VTable(guid, ScriptBinder<CPrototypeTest>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\CScript_Test.script");
        handler.Register_VTable(guid, ScriptBinder<CScript_Test>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\Test_ComponentEnable.script");
        handler.Register_VTable(guid, ScriptBinder<CTest_ComponentEnable>::Build());
    }
}
NS_END