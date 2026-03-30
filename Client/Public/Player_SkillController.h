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
    void                Update_SkillSet();

    _bool               Try_UseSKill(SKILL eSkill);

    SKILL               Get_CurSkill() const;
    _float              Get_ElapsedCoolDown(SKILL eSkill) const;
    _float              Get_TotalCoolDown(SKILL eSkill) const;

private:
    PLAYER_SKILLSET*    m_pSkillSet{};
    CEvent<SKILL>       m_OnCoolDownFinished[To<_uint>(SKILL::END)];

public:
    static std::unique_ptr<CPlayer_SkillController> Create(PLAYER_SKILLSET* pSkillSet);

public:
    template<typename T>
    void Subscribe_OnSkillCooldownFinished(void(T::* func)(SKILL), T* pInstance, SKILL eSkill)
    {
        return m_OnCoolDownFinished[To<_uint>(eSkill)].Add_Listener(func, pInstance);
    }

};

NS_END
