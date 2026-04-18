#pragma once
#include "State.h"
#include "Player_Struct.h"

NS_BEGIN(Client)

class CPlayer;
class CPlayerStateMachine;
class CTrail;

class CPlayerState : public CState
{

public :
    CPlayerState(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : m_goPlayer(goPlayer) , m_scPlayer(scPlayer), m_eState(eState)
    {
        std::string_view strState = magic_enum::enum_name(eState);

        strncpy_s(m_szStateName, sizeof(m_szStateName), strState.data(), _TRUNCATE);
    }
    ~CPlayerState() = default;

public :
    virtual HRESULT Initialize();
    virtual void    Priority_Update(_float fDT);;
    virtual void    Update(_float fDT);
    virtual void    Late_Update(_float fDT);;

    void            Enter(_uint iDetailFlag) override;
    void            Exit() override;

    virtual void    Control_Camera();
    virtual void    Update_PlayerInput(const PLAYER_INPUT_COMMAND& tInputCmd);
    virtual void    Setup_CachedPlayerContext();

    /* commons */
    void            LookTo_InputDir(_float fDT);
    void            LookTo_AnchorPos(_float fDT);
    void            Try_Grappling();
    void            Finish_Grappling();
    void            GroundedMove(_float fDT);
    void            Handle_Trail(_float fDT, WIDTH_TYPE eWidth);
    void            Handle_SpeedLines(_float fDT);
    _bool           Set_HitBoxActive(const std::string& strHitBox, _bool bActive);
    void            Sync_HiBox(const std::string& strHitBox);

    virtual void    Cache_PlayerContext(const PLAYER_CONTEXT& tContext);
    PLAYER_STATE    Get_State() const;
    const char*     Get_StateName() const;

private :
    virtual void    Decide_NextState() {};
    virtual void    Decide_NextAnim() {};

protected:
    Engine::CGameObject*    m_goPlayer{};
    CPlayer*                m_scPlayer{};
    CTrail*                 m_pTrail{};

    PLAYER_INPUT_COMMAND    m_tInputCmd{};
    PLAYER_COMPONENTS       m_tComponents{};
    PLAYER_RUNTIME_REF      m_tRef{};
    PLAYER_STATS*           m_pStats{};
    BLADE_DURABILITY*       m_pBlade{};

    class CPlayer_SkillController*  m_pSkillController{};

protected :
    _bool                   m_bAcivated = false;

    /* direction by camera */
    _float3                 m_vPrevLook{};
    _float                  m_fRotateSharpness = 10.f;
    _float                  m_fCurrentYaw = 0.f;
    _bool                   m_bYawInitialized = false;

    PLAYER_STATE            m_eState = PLAYER_STATE::IDLE;
    _char                   m_szStateName[32];

    _float3                 m_vHitBoxOffset = { 0.2f, 0.8f, 0.8f };

};

NS_END
