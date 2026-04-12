#pragma once
#include "Entity_Define.h"

NS_BEGIN(Client)

enum class EREN_STEP_TYPE : _uint
{
    NONE,
    BORNE,
    COMBAT,
    MOVE_TO,
    LIFT_ROCK,
    MOVE_ROCK,
    FIX_ROCK,
    PLAY_ANIM,
    END
};

typedef struct  tagErenDirectorStep final
{
    EREN_STEP_TYPE      eType = EREN_STEP_TYPE::NONE;
    _float3             vTargetPos = {};
    _float              fDuration = 0.f;
    vector<std::string> szAnims;
} EREN_DIRECTOR_STEP;


enum class EREN_BORN { AIR_FALL, CAN_LAND, LAND, CAN_ROAR, ROAR, END };
enum class EREN_COMBAT {
    WAIT,
    APPROACHING,
    KICK,
    COMBO1,         /* right straight punch */
    COMBO2,         /* Left Uppercut */
    COMBO3,         /* right Elbow Strike */
    FULL_COMBO,
    END };


typedef struct tagErenCombatPattern
{
    EREN_COMBAT eCombatType = EREN_COMBAT::END;
    _float      fKeepDistance = 0.f;
} EREN_COMBAT_PATTERN;

inline constexpr const char* HAND_L = "hand_L";
inline constexpr const char* HAND_R = "hand_R";
inline constexpr const char* LEG_L = "leg_L";

NS_END
