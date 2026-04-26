#pragma once
#include "State.h"
#include "Titan_Struct.h"

NS_BEGIN(Client)

class CTitanState : public CState
{
protected:
    CTitanState(Engine::CGameObject* goPlayer, CTitan* scPlayer, TITAN_STATE eState)
        : m_goTitan(goPlayer), m_scTitan(scPlayer), m_eState(eState)
    {
        std::string_view strState = magic_enum::enum_name(eState);

        strncpy_s(m_szStateName, sizeof(m_szStateName), strState.data(), _TRUNCATE);
    }
    ~CTitanState() = default;

public:
    virtual HRESULT Initialize();
    virtual void    Priority_Update(_float fDT);
    virtual void    Update(_float fDT);
    virtual void    Late_Update(_float fDT);

    void            Enter(_uint iDetailFlag) override;
    void            Exit() override;

    virtual void    Cache_TitanContext(const TITAN_CONTEXT& tContext);
    virtual void    Setup_CachedTitanContext();

    /* commons */
    virtual _vector Get_PatrolMoveDir();
    void            GroundedMove(_fvector vDir, _float fDT);
    void            Look_To(_fvector vDir, _float fDT);
    void            Detect_Human();
    void            Chase_Human();

    /* Debug */
    TITAN_STATE     Get_State() const;
    const char*     Get_StateName() const;
    virtual _uint   Get_DetailState() const;

private:
    virtual void    Decide_NextState() {};
    virtual void    Decide_NextAnim() {};

protected:
    Engine::CGameObject*    m_goTitan{};
    CTitan*                 m_scTitan{};

    TITAN_COMPONENTS        m_tComponents{};
    TITAN_RUNTIME_REF       m_tRef{};
    TITAN_STATS*            m_pStats{};
    PATROL_INFO*            m_pPatrol{};
    wstring                 m_wstrSFX = L"";

protected:
    _bool                   m_bAcivated = false;

    /* direction to Player */
    _float3                 m_vPrevLook{};
    _float                  m_fRotateSharpness = 0.5f;
    _float                  m_fCurrentYaw = 0.f;
    _bool                   m_bYawInitialized = false;

    TITAN_STATE             m_eState = TITAN_STATE::IDLE;
    _char                   m_szStateName[32];
};

NS_END
