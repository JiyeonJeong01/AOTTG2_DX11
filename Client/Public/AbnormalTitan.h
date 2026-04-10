#pragma once
#include "Client_Define.h"
#include "Script.h"

#include "Titan_Struct.h"

NS_BEGIN(Client)
class CAbnormalTitanStateMachine;
class CTargetSensor;
class CTitanState;
NS_END

NS_BEGIN(Client)

class CAbnormalTitan : public IScript, public CTitan
{
public:
    char        m_szState[32] = {};

    SCRIPT_FIELDS_BEGIN(CAbnormalTitan)
        SCRIPT_FIELD_CHAR(m_szState)
    SCRIPT_FIELDS_END(CAbnormalTitan)

public:
    CAbnormalTitan();
    ~CAbnormalTitan();

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private:
    CGameObject* m_goTitan = nullptr;
    CGameObject* m_goTarget = nullptr;

    TITAN_COMPONENTS        m_tComponents{};
    TITAN_RUNTIME_REF       m_tRef{};
    TITAN_STATS             m_tStats{};
    TITAN_POSE              m_ePose = TITAN_POSE::END;

    std::unique_ptr<CAbnormalTitanStateMachine>   m_upStateMachine{};
    std::shared_ptr<CTitanState>                m_spCurState{};

    std::unordered_map<std::string, CHitBox*>   m_AllHitBoxes;

    Engine::CEvent<Engine::CGameObject*>        m_OnChanged_Target;

public:
    TITAN_CONTEXT Get_TitanContext();

    template <typename T>
    ListenerID Subscribe_OnChangedTarget(void(T::* func)(Engine::CGameObject*), T* pInstance)
    {
        return m_OnChanged_Target.Add_Listener(func, pInstance);
    }

    void                Set_Target(Engine::CGameObject* pTarget);
    void                Clear_Target();
    void                Validate_Target();

    _bool               Has_Target() const;
    _bool               Is_ValidTarget(Engine::CGameObject* pTarget);

    CGameObject* Get_CurTarget() const;

private:
    void On_Grab(SIDE eSide, CHuman* pHuman) override;
    void On_Dead(const _float fAccuracy) override;
    void On_Stunned() override;
    void On_Hurt(const HIT_INFO& tHitBox, const std::string& strHurtBox);

    void On_DetectedHumanSide(CGameObject* goHuman);
    void OnChange_CurState(std::shared_ptr<CTitanState> spNewState);
};


NS_END;
