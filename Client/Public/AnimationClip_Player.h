#pragma once

namespace ANIM_PLAYER
{
    /* IdleState */
    inline constexpr const char* IDLE_F = "Armature|idle_F";
    inline constexpr const char* IDLE_M = "Armature|idle_M";
    inline constexpr const char* IDLE_TS_F = "Armature|idle_TS_F";
    inline constexpr const char* IDLE_TS_M = "Armature|idle_TS_M";
    inline constexpr const char* IDLE_CASUAL_F = "Armature|idle_casual_F";
    inline constexpr const char* IDLE_CASUAL_M = "Armature|idle_casual_M";
    inline constexpr const char* IDLE_ACT_1 = "Armature|idle_action_1";
    inline constexpr const char* IDLE_ACT_2 = "Armature|idle_action_2";

    /* MoveState */
    inline constexpr const char* RUN = "Armature|run";
    inline constexpr const char* RUN_TS = "Armature|run_TS";
    inline constexpr const char* RUN_CASUAL = "Armature|run_casual";
    inline constexpr const char* RUN_SASHA = "Armature|run_sasha";
    inline constexpr const char* DASH = "Armature|dash";
    inline constexpr const char* SLIDE = "Armature|slide";
    inline constexpr const char* DASH_LAND = "Armature|dash_land";

    /* JumpState */
    inline constexpr const char* JUMP = "Armature|jump";

    /* AirborneState */
    inline constexpr const char* AIR = "Armature|air2";
    inline constexpr const char* AIR_BACKWARD = "Armature|air2_backward";
    inline constexpr const char* AIR_LEFT = "Armature|air2_left";
    inline constexpr const char* AIR_RIGHT = "Armature|air2_right";
    inline constexpr const char* AIR_TS = "Armature|air2_ts";
    inline constexpr const char* AIR_CIRCLE = "Armature|air_circle";
    inline constexpr const char* AIR_FALL = "Armature|air_fall";
    inline constexpr const char* AIR_RELEASE = "Armature|air_release";
    inline constexpr const char* AIR_RISE = "Armature|air_rise";

    /* HookState */
    inline constexpr const char* AIR_HOOK = "Armature|air_hook";
    inline constexpr const char* AIR_HOOK_L = "Armature|air_hook_l";
    inline constexpr const char* AIR_HOOK_L_JUST = "Armature|air_hook_l_just";
    inline constexpr const char* AIR_HOOK_R = "Armature|air_hook_r";
    inline constexpr const char* AIR_HOOK_R_JUST = "Armature|air_hook_r_just";
    inline constexpr const char* ON_WALL = "Armature|onWall";
    inline constexpr const char* TO_ROOF = "Armature|toRoof";
    inline constexpr const char* WALL_RUN = "Armature|wallrun";

    /* AttackState */
    inline constexpr const char* ATTACK_1 = "Armature|attack1";
    inline constexpr const char* ATTACK_1_HOOK_L1 = "Armature|attack1_hook_l1";
    inline constexpr const char* ATTACK_1_HOOK_L2 = "Armature|attack1_hook_l2";
    inline constexpr const char* ATTACK_1_HOOK_R1 = "Armature|attack1_hook_r1";
    inline constexpr const char* ATTACK_1_HOOK_R2 = "Armature|attack1_hook_r2";
    inline constexpr const char* ATTACK_2 = "Armature|attack2";
    inline constexpr const char* ATTACK_4 = "Armature|attack4";
    inline constexpr const char* ATTACK_3_1 = "Armature|attack_3_1";
    inline constexpr const char* ATTACK_3_2 = "Armature|attack_3_2";

    /* ShootState */
    inline constexpr const char* APG_SHOOT_L = "Armature|APG_shoot_L";
    inline constexpr const char* APG_SHOOT_R = "Armature|APG_shoot_R";
    inline constexpr const char* APG_SHOOT_BOTH = "Armature|APG_shooth_both";
    inline constexpr const char* TS_SHOOT_L = "Armature|TS_shoot_L";
    inline constexpr const char* TS_SHOOT_R = "Armature|TS_shoot_R";
    inline constexpr const char* TS_SHOOT_AIR_L = "Armature|TS_shoot_air_L";
    inline constexpr const char* TS_SHOOT_AIR_R = "Armature|TS_shoot_air_R";

    /* ReloadState
       실제로는 BladeChangeState 용도로 같이 사용 */
    inline constexpr const char* CHANGE_BLADE = "Armature|changeBlade";
    inline constexpr const char* CHANGE_BLADE_AIR = "Armature|changeBlade_air";
    inline constexpr const char* APG_RELOAD_BOTH = "Armature|APG_reload_both";
    inline constexpr const char* APG_RELOAD_L = "Armature|APG_reload_L";
    inline constexpr const char* APG_RELOAD_R = "Armature|APG_reload_R";
    inline constexpr const char* APG_RELOAD_AIR_BOTH = "Armature|APG_reload_air_both";
    inline constexpr const char* APG_RELOAD_AIR_L = "Armature|APG_reload_air_L";
    inline constexpr const char* APG_RELOAD_AIR_R = "Armature|APG_reload_air_R";

    /* DodgeState */
    inline constexpr const char* DODGE = "Armature|dodge";

    /* ResupplyState */
    inline constexpr const char* RESUPPLY = "Armature|resupply";

    /* HorseState */
    inline constexpr const char* HORSE_GET_ON = "Armature|horse_geton";
    inline constexpr const char* HORSE_GET_OFF = "Armature|horse_getoff";
    inline constexpr const char* HORSE_IDLE = "Armature|horse_idle";
    inline constexpr const char* HORSE_RUN = "Armature|horse_run";

    /* GrabbedState */
    inline constexpr const char* GRABBED = "Armature|grabbed";
    inline constexpr const char* GRABBED_JEAN = "Armature|grabbed_jean";

    /* EmoteState */
    inline constexpr const char* EMOTE_NO = "Armature|emote_no";
    inline constexpr const char* EMOTE_SALUTE = "Armature|emote_salute";
    inline constexpr const char* EMOTE_WAVE = "Armature|emote_wave";
    inline constexpr const char* EMOTE_YES = "Armature|emote_yes";

    /* SpecialActionState */
    inline constexpr const char* CLOAK_2 = "Armature|cloak 2|cloak 2|YP|Take 001|BaseLayer";
    inline constexpr const char* PROMOTIONAL_POSES = "Armature|promotionl_poses";
    inline constexpr const char* SPECIAL_LEVI = "Armature|special_levi";
    inline constexpr const char* SPECIAL_PETRA = "Armature|special_petra";
    inline constexpr const char* SPECIAL_SASHA = "Armature|special_sasha";
    inline constexpr const char* SPECIAL_SHIFT_0 = "Armature|special_shift_0";
    inline constexpr const char* SPECIAL_SHIFT_1 = "Armature|special_shift_1";
    inline constexpr const char* SPECIAL_ARMIN = "Armature|special_armin";

    /* DebugOrEditorState */
    inline constexpr const char* T_POSE = "T-pose";
    inline constexpr const char* ARMATURE_T_POSE = "Armature|T-pose";
    inline constexpr const char* TEST_POSE = "Armature|test_pose";
}
