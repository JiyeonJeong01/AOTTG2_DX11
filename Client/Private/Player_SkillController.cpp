#include "Player_SkillController.h"

CPlayer_SkillController::CPlayer_SkillController(PLAYER_SKILLSET* pSkillSet)
    : m_pSkillSet(pSkillSet)
{
}

CPlayer_SkillController::~CPlayer_SkillController()
{
}

void CPlayer_SkillController::SetUp_SkillSet()
{
    PLAYER_SKILL skill1;
    skill1.eSkill = SKILL::SPIN_H;
    skill1.strName = "수평 베기";
    skill1.fCoolDown = 0.f;//17.f;
    skill1.fElapsedCoolDown = 0.f;
    skill1.tSpriteGUID = ASSET_GUID("C212BB34-F7FE-440A-9A89-348437E93C4A");

    PLAYER_SKILL skill2;
    skill2.eSkill = SKILL::THROW;
    skill2.strName = "칼날 던지기";
    skill2.fCoolDown = 10.f;
    skill2.fElapsedCoolDown = 0.f;
    skill2.tSpriteGUID = ASSET_GUID("29085E95-8FA2-4160-A444-EDE76E0F9B34");

    PLAYER_SKILL skill3;
    skill3.eSkill = SKILL::SPIN_V;
    skill3.strName = "수직 베기";
    skill3.fCoolDown = 17.f;
    skill3.fElapsedCoolDown = 0.f;
    skill3.tSpriteGUID = ASSET_GUID("43909CC7-975A-4F95-8328-1C13BD927403");

    m_pSkillSet->skills[0] = skill1;
    m_pSkillSet->skills[1] = skill2;
    m_pSkillSet->skills[2] = skill3;

    m_pSkillSet->eSkill = SKILL::SPIN_H;
}

void CPlayer_SkillController::Update_SkillSet(_float fDT)
{
    /* 쿨다운 업데이트 */
    for(auto& skill : m_pSkillSet->skills)
    {
        skill.fElapsedCoolDown += fDT;
        skill.fElapsedCoolDown = fminf(skill.fElapsedCoolDown, skill.fCoolDown);
    }
}

_bool CPlayer_SkillController::Try_UseSKill(SKILL eSkill)
{
    _uint iSkillIdx = To<_uint>(eSkill);
    auto& skill = m_pSkillSet->skills[iSkillIdx];

    if (skill.fCoolDown > skill.fElapsedCoolDown)
        return false;
    
    skill.fElapsedCoolDown = 0.f;
    return true;
}

SKILL CPlayer_SkillController::Get_CurSkill() const
{
    return m_pSkillSet->eSkill;
}

_float CPlayer_SkillController::Get_ElapsedCoolDown(SKILL eSkill) const
{
    _uint iSkillIdx = To<_uint>(eSkill);
    auto& skill = m_pSkillSet->skills[iSkillIdx];

    return skill.fElapsedCoolDown;
}

_float CPlayer_SkillController::Get_TotalCoolDown(SKILL eSkill) const
{
    _uint iSkillIdx = To<_uint>(eSkill);
    auto& skill = m_pSkillSet->skills[iSkillIdx];

    return skill.fCoolDown;
}

std::unique_ptr<CPlayer_SkillController> CPlayer_SkillController::Create(PLAYER_SKILLSET* pSkillSet)
{
    return std::make_unique<CPlayer_SkillController>(pSkillSet);
}
