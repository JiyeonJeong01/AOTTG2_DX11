#pragma once
#include "Client_Define.h"
#include "Script.h"

#include "Titan_Struct.h"

NS_BEGIN(Engine)
class CNavMesh;
NS_END

NS_BEGIN(Client)
class CUI_HitController;
class CVFX_Manager;
class CNormalTitanStateMachine;
class CTargetSensor;
class CTitanState;
NS_END

NS_BEGIN(Client)

class CNormalTitan : public IScript, public CTitan
{
public:
    CNormalTitan();
    ~CNormalTitan();

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    CVFX_Manager*       m_pVFX_Manager = nullptr;
    CUI_HitController*  m_pUIHitController = nullptr;

    CGameObject*            m_goTitan = nullptr;
    CGameObject*            m_goTarget = nullptr;
    CGameObject*            m_goWeakPoint = nullptr;

    TITAN_COMPONENTS        m_tComponents{};
    TITAN_RUNTIME_REF       m_tRef{};
    TITAN_STATS             m_tStats{};
    TITAN_POSE              m_ePose = TITAN_POSE::END;
    PATROL_INFO             m_tPatrol{};
    TITAN_DUST_RUNTIME      m_tDustRuntime{};

    _uint                   m_iStunnedAcc = 0;
    _int                    m_iHitEffect = 0;

    std::unique_ptr<CNormalTitanStateMachine>   m_upStateMachine{};
    std::shared_ptr<CTitanState>                m_spCurState{};
    std::unique_ptr<CNavMesh>                   m_upNav{};

    std::unordered_map<std::string, CHitBox*>   m_AllHitBoxes;
    vector<CMeshRenderer>   m_MrChildren;

    Engine::CEvent<Engine::CGameObject*>        m_OnChanged_Target;

public :
    TITAN_CONTEXT Get_TitanContext();
    TITAN_TYPE Get_TitanType() const override { return TITAN_TYPE::NORMAL; }

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

    CGameObject*        Get_CurTarget() const;
    _bool               Is_Moving() override;

private:
    void On_Grab(SIDE eSide, CHuman* pHuman) override;
    void On_Dead(const _float fAccuracy) override;
    void On_Stunned(const HIT_INFO& tHitInfo) override;
    void On_Hurt(const HIT_INFO& tHitBox, const std::string& strHurtBox);

    void On_DetectedHumanSide(CGameObject* goHuman);
    void OnChange_CurState(std::shared_ptr<CTitanState> spNewState);

    void Set_FootDust();
    void Update_FootDust();
    void Start_Dissolve();
    void Update_Dissolve(_float fDT);

private :
    char        m_szState[32] = {};
    SCRIPT_OBJECT_REF   m_refVFXManager{};
    SCRIPT_OBJECT_REF   m_refUIHit{};

    SCRIPT_FIELDS_BEGIN(CNormalTitan)
        SCRIPT_FIELD_CHAR(m_szState)
        SCRIPT_FIELD_FLOAT3(m_tPatrol.vPos[0]);
        SCRIPT_FIELD_FLOAT3(m_tPatrol.vPos[1]);
        SCRIPT_FIELD_OBJECT_REF(m_refVFXManager);
        SCRIPT_FIELD_OBJECT_REF(m_refUIHit);
    SCRIPT_FIELDS_END(CNormalTitan)
};


NS_END;
