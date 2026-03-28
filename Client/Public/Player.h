#pragma once

#include "Client_Define.h"
#include "Player_Struct.h"

NS_BEGIN(Client)
class CPlayer_InputController;
class CPlayerStateMachine;
class CPlayerState;
class CCameraController;
class CODM_Gear;
NS_END

NS_BEGIN(Client)
class CPlayer : public IScript
{

public:
    _float  m_fSpeed = 0;

public:
    SCRIPT_FIELDS_BEGIN(CPlayer)
        SCRIPT_FIELD_FLOAT(m_fSpeed)
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

    std::unique_ptr<CPlayer_InputController>    m_upInputController{};
    std::unique_ptr<CPlayerStateMachine>        m_upStateMachine{};
    std::shared_ptr<CPlayerState>               m_spCurState{};

    CCameraController*                          m_pCameraController{};
    CODM_Gear*                                  m_pGear{};

private :
    void OnChange_CurState(std::shared_ptr<CPlayerState> spNewState);
};

NS_END;
