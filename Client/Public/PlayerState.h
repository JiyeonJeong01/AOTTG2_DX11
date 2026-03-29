#pragma once
#include "State.h"
#include "Player_Struct.h"

NS_BEGIN(Client)

class CPlayer;
class CPlayerStateMachine;

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

    virtual void    Control_Camera();
    virtual void    Update_PlayerInput(const PLAYER_INPUT_COMMAND& tInputCmd);
    virtual void    Setup_CachedPlayerInfos();

    void            LookTo_InputDir(_float fDT);

    void            Cache_PlayerInfos(const PLAYER_COMPONENTS& tComponents, const PLAYER_RUNTIME_REF& tRef, PLAYER_INFO* pInfo);
    void            Bind_PlayerRef(const PLAYER_RUNTIME_REF& tRef);
    PLAYER_STATE    Get_State() const;
    const char*     Get_StateName() const;

private :
    virtual void    Decide_NextState() {};

protected:
    Engine::CGameObject*    m_goPlayer{};
    CPlayer*                m_scPlayer{};
    CPlayerStateMachine*    m_pFSM{};

    PLAYER_INPUT_COMMAND    m_tInputCmd{};
    PLAYER_COMPONENTS       m_tComponents{};
    PLAYER_RUNTIME_REF      m_tRef{};
    PLAYER_INFO*            m_pInfo{};

private :
    _float3                 m_vPrevLook{};
    _float                  m_fRotateSharpness = 10.f;
    _float                  m_fCurrentYaw = 0.f;
    _bool                   m_bYawInitialized = false;

    PLAYER_STATE            m_eState = PLAYER_STATE::IDLE;
    _char                   m_szStateName[32];
};

NS_END
