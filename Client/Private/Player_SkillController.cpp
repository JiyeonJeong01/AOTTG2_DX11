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
    skill1.eSkill = SKILL_TYPE::SPIN_H;
    skill1.strName = "수평 베기";
    skill1.fCoolDown = 5.f;
    skill1.fElapsedCoolDown = skill1.fCoolDown;
    skill1.tSpriteGUID = ASSET_GUID("C212BB34-F7FE-440A-9A89-348437E93C4A");

    PLAYER_SKILL skill2;
    skill2.eSkill = SKILL_TYPE::THROW;
    skill2.strName = "칼날 던지기";
    skill2.fCoolDown = 5.f;
    skill2.fElapsedCoolDown = skill2.fCoolDown;
    skill2.tSpriteGUID = ASSET_GUID("29085E95-8FA2-4160-A444-EDE76E0F9B34");

    PLAYER_SKILL skill3;
    skill3.eSkill = SKILL_TYPE::SPIN_V;
    skill3.strName = "수직 베기";
    skill3.fCoolDown = 5.f;
    skill3.fElapsedCoolDown = skill3.fCoolDown;
    skill3.tSpriteGUID = ASSET_GUID("43909CC7-975A-4F95-8328-1C13BD927403");

    m_pSkillSet->skills[0] = skill1;
    m_pSkillSet->skills[1] = skill2;
    m_pSkillSet->skills[2] = skill3;

    m_pSkillSet->eCurSkill = SKILL_TYPE::SPIN_H;
}

void CPlayer_SkillController::Update_SkillSet(_float fDT, const PLAYER_INPUT_COMMAND& tInputCmd)
{
    /* 쿨다운 업데이트 */
    for(auto& skill : m_pSkillSet->skills)
    {
        skill.fElapsedCoolDown += fDT;
        if (skill.fElapsedCoolDown >= skill.fCoolDown)
        {
            /* 클램프 후 이벤트 호출 */
            skill.fElapsedCoolDown = skill.fCoolDown;
            if (!skill.bCoolDownCompleted)
            {
                skill.bCoolDownCompleted = true;
                m_OnCoolDownCompleted.Invoke(skill.eSkill);
            }
        }
    }

    _int iStep = tInputCmd.iSwitchSkillDir / 120;
    if (iStep != 0)
    {
        /* 활성화된 스킬 업데이트 */
        /* [+] : => */
        /* [-] : <= */
        _uint iNumSkills = To<_uint>(SKILL_TYPE::END);
        _uint iSkillIdx = (To<_uint>(m_pSkillSet->eCurSkill) + iStep + iNumSkills) % iNumSkills;

        if (m_ePrevSkillType != m_pSkillSet->eCurSkill)
        {
            m_ePrevSkillType = m_pSkillSet->eCurSkill;
            m_pSkillSet->eCurSkill = To<SKILL_TYPE>(iSkillIdx);
            m_OnActivatedSkillChanged.Invoke(m_pSkillSet->eCurSkill);
        }
    }
}

_bool CPlayer_SkillController::Try_UseSKill(SKILL_TYPE eSkill)
{
    _uint iSkillIdx = To<_uint>(eSkill);
    auto& skill = m_pSkillSet->skills[iSkillIdx];

    if (skill.fCoolDown > skill.fElapsedCoolDown)
        return false;
    
    skill.fElapsedCoolDown = 0.f;

    m_OnSkillUsed.Invoke(skill.eSkill);
    return true;
}

PLAYER_SKILL* CPlayer_SkillController::Get_SkillPtr(SKILL_TYPE eType)
{
    return &(m_pSkillSet->skills[To<_uint>(eType)]);
}

PLAYER_SKILL* CPlayer_SkillController::Get_CurSkillPtr()
{
    return &(m_pSkillSet->skills[To<_uint>(m_pSkillSet->eCurSkill)]);
}

SKILL_TYPE CPlayer_SkillController::Get_CurSkillType() const
{
    return m_pSkillSet->eCurSkill;
}

_float CPlayer_SkillController::Get_ElapsedCoolDown(SKILL_TYPE eSkill) const
{
    _uint iSkillIdx = To<_uint>(eSkill);
    auto& skill = m_pSkillSet->skills[iSkillIdx];

    return skill.fElapsedCoolDown;
}

_float CPlayer_SkillController::Get_TotalCoolDown(SKILL_TYPE eSkill) const
{
    _uint iSkillIdx = To<_uint>(eSkill);
    auto& skill = m_pSkillSet->skills[iSkillIdx];

    return skill.fCoolDown;
}

std::unique_ptr<CPlayer_SkillController> CPlayer_SkillController::Create(PLAYER_SKILLSET* pSkillSet)
{
    return std::make_unique<CPlayer_SkillController>(pSkillSet);
}
