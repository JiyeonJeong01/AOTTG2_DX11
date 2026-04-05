// AUTO-GENERATED. DO NOT EDIT.
#include "Script_Registry.h"
#include "Asset_Registry.h"
#include "Script_Handler.h"
#include "Script_Register.h"

#include "CameraController.h"
#include "FreeCam.h"
#include "GameManager.h"
#include "GroundChecker.h"
#include "HUDController.h"
#include "Hello.h"
#include "HitBox.h"
#include "HurtBox.h"
#include "MainMenu_Controller.h"
#include "NormalTitan.h"
#include "ODM_Gear.h"
#include "Player.h"
#include "Scout.h"
#include "TargetSensor.h"
#include "ThrownBlade.h"
#include "TitanBound_Controller.h"
#include "Titan_Hand.h"
#include "UI_BladeController.h"
#include "UI_GasController.h"
#include "UI_SkillController.h"

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
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\HUDController.script");
        handler.Register_VTable(guid, ScriptBinder<CHUDController>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\Hello.script");
        handler.Register_VTable(guid, ScriptBinder<CHello>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\HitBox.script");
        handler.Register_VTable(guid, ScriptBinder<CHitBox>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\HurtBox.script");
        handler.Register_VTable(guid, ScriptBinder<CHurtBox>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\MainMenu_Controller.script");
        handler.Register_VTable(guid, ScriptBinder<CMainMenu_Controller>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\NormalTitan.script");
        handler.Register_VTable(guid, ScriptBinder<CNormalTitan>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\ODM_Gear.script");
        handler.Register_VTable(guid, ScriptBinder<CODM_Gear>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\Player.script");
        handler.Register_VTable(guid, ScriptBinder<CPlayer>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\Scout.script");
        handler.Register_VTable(guid, ScriptBinder<CScout>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\TargetSensor.script");
        handler.Register_VTable(guid, ScriptBinder<CTargetSensor>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\ThrownBlade.script");
        handler.Register_VTable(guid, ScriptBinder<CThrownBlade>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\TitanBound_Controller.script");
        handler.Register_VTable(guid, ScriptBinder<CTitanBound_Controller>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\Titan_Hand.script");
        handler.Register_VTable(guid, ScriptBinder<CTitan_Hand>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\UI_BladeController.script");
        handler.Register_VTable(guid, ScriptBinder<CUI_BladeController>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\UI_GasController.script");
        handler.Register_VTable(guid, ScriptBinder<CUI_GasController>::Build());
    }
    {
        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path("C:\\Users\\delay\\Jusin\\AOTTG2_DX11\\Client\\Bin\\Assets\\Scripts\\UI_SkillController.script");
        handler.Register_VTable(guid, ScriptBinder<CUI_SkillController>::Build());
    }
}
NS_END