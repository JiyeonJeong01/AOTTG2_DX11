#pragma once

#include "Client_Define.h"
#include "Player_Struct.h"

NS_BEGIN(Client)

class CPlayer_SkillController final
{
public:
    CPlayer_SkillController(PLAYER_SKILLSET* pSkillSet);
    ~CPlayer_SkillController();

public:
    void                SetUp_SkillSet();
    void                Update_SkillSet(_float fDT, const PLAYER_INPUT_COMMAND& tInputCmd);

    _bool               Try_UseSKill(SKILL_TYPE eSkill);

    PLAYER_SKILL*       Get_SkillPtr(SKILL_TYPE eType);
    PLAYER_SKILL*       Get_CurSkillPtr();
    SKILL_TYPE          Get_CurSkillType() const;
    _float              Get_ElapsedCoolDown(SKILL_TYPE eSkill) const;
    _float              Get_TotalCoolDown(SKILL_TYPE eSkill) const;

private:
    PLAYER_SKILLSET*    m_pSkillSet{};
    SKILL_TYPE          m_ePrevSkillType = SKILL_TYPE::END;

    CEvent<SKILL_TYPE>  m_OnCoolDownCompleted;
    CEvent<SKILL_TYPE>  m_OnSkillUsed;
    CEvent<SKILL_TYPE>  m_OnActivatedSkillChanged;

public:
    static std::unique_ptr<CPlayer_SkillController> Create(PLAYER_SKILLSET* pSkillSet);

public:
    template<typename T>
    ListenerID Subscribe_OnSkillCooldownCompleted(void(T::* func)(SKILL_TYPE), T* pInstance)
    {
        return m_OnCoolDownCompleted.Add_Listener(func, pInstance);
    }

    template<typename T>
    ListenerID Subscribe_OnSkillUsed(void(T::* func)(SKILL_TYPE), T* pInstance)
    {
        return m_OnSkillUsed.Add_Listener(func, pInstance);
    }

    template<typename T>
    ListenerID Subscribe_OnActivatedSkillChanged(void(T::* func)(SKILL_TYPE), T* pInstance)
    {
        return m_OnActivatedSkillChanged.Add_Listener(func, pInstance);
    }

};

NS_END
