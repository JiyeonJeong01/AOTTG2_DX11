#pragma once

#include "Client_Define.h"
#include "Player_Struct.h"

NS_BEGIN(Client)
class CPlayer_InputController;
class CPlayer_SkillController;
class CUI_HitController;
class CPlayerStateMachine;
class CPlayerState;
class CCameraController;
class CODM_Gear;
class CTrail;
NS_END

NS_BEGIN(Client)
class CPlayer : public IScript, public CHuman
{

public:
    _float              m_fSpeed = 0;
    char                m_szState[32] = {};
    _float              m_fForceDrag = 0.5f;

    SCRIPT_OBJECT_REF   m_refVFXManager{};
    SCRIPT_OBJECT_REF   m_refUIHit{};

public:
    SCRIPT_FIELDS_BEGIN(CPlayer)
        SCRIPT_FIELD_CHAR(m_szState)
        SCRIPT_FIELD_FLOAT(m_fSpeed)
        SCRIPT_FIELD_FLOAT(m_fForceDrag)
        SCRIPT_FIELD_OBJECT_REF(m_refVFXManager);
        SCRIPT_FIELD_OBJECT_REF(m_refUIHit);
    SCRIPT_FIELDS_END(CPlayer)

public :
    CPlayer();
    ~CPlayer();

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private:
    CGameObject*        m_goPlayer = nullptr;
    PLAYER_COMPONENTS   m_tComponents{};
    PLAYER_RUNTIME_REF  m_tRef{};
    PLAYER_STATS        m_tStats{};
    PLAYER_SKILLSET     m_tSkillSet{};
    BLADE_DURABILITY    m_tBlade{};

    PLAYER_CONTEXT      m_tContext{};
    CVFX_Manager*       m_pVFX_Manager = nullptr;
    CUI_HitController*  m_pUIHitController = nullptr;

    std::unique_ptr<CPlayer_InputController>    m_upInputController{};
    std::unique_ptr<CPlayer_SkillController>    m_upSkillController{};
    std::unique_ptr<CPlayerStateMachine>        m_upStateMachine{};
    std::shared_ptr<CPlayerState>               m_spCurState{};
    std::unique_ptr<CTrail>                     m_upTrail{};

    CCameraController*                          m_pCameraController{};
    CODM_Gear*                                  m_pGear{};
    CGameObject*                                m_goGasResupply{};

    std::unordered_map<std::string, class CHitBox*> m_AllHitBoxes;

private :
    void On_Grabbed(SIDE eSide, CTitan* pTitan) override;
    void On_Dead() override;

    void OnChange_CurState(std::shared_ptr<CPlayerState> spNewState);
    void On_BladeHit(CGameObject* goCounter, const HIT_INFO& tHitInfo);
    void On_DetectedTitan(CGameObject* goTitan);

public :
    PLAYER_CONTEXT Get_PlayerContext();
    void Resupply();
    void Ready_Deliver_Supplies();
    void Complete_Deliver_Supplies();

private :
    void Display_GasResupply(_bool bDisplay);

    void Set_ReferenceObject();
    void Set_ReferenceComponent();
    void Set_ReferenceScript();
    void Build_Context();
};


NS_END;
