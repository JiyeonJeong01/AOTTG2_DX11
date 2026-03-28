// AUTO-GENERATED. DO NOT EDIT.
#include "Script_Registry.h"
#include "Asset_Registry.h"
#include "Script_Handler.h"
#include "Script_Register.h"

#include "CameraController.h"
#include "FreeCam.h"
#include "GameManager.h"
#include "GroundChecker.h"
#include "Hello.h"
#include "MainMenu_Controller.h"
#include "ODM_Gear.h"
#include "Player.h"

NS_BEGIN(Client)
void Register_AllScripts()
{
    auto& handler = SYS_ASSET.Scripts();
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\CameraController.script");
        handler.Register_VTable(guid, ScriptBinder<CCameraController>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\FreeCam.script");
        handler.Register_VTable(guid, ScriptBinder<CFreeCam>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\GameManager.script");
        handler.Register_VTable(guid, ScriptBinder<CGameManager>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\GroundChecker.script");
        handler.Register_VTable(guid, ScriptBinder<CGroundChecker>::Build());
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
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\ODM_Gear.script");
        handler.Register_VTable(guid, ScriptBinder<CODM_Gear>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\Player.script");
        handler.Register_VTable(guid, ScriptBinder<CPlayer>::Build());
    }
}
NS_END