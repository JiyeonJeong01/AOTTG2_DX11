#pragma once
#include "Titan_Struct.h"

NS_BEGIN(Client)

class CTargetSensor final : public IScript
{
public :
    CTargetSensor();
    ~CTargetSensor();

    SCRIPT_OBJECT_REF   m_refTitan{};

    SCRIPT_FIELDS_BEGIN(CTargetSensor)
        SCRIPT_FIELD_OBJECT_REF(m_refTitan)
    SCRIPT_FIELDS_END(CTargetSensor)

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private:
    Engine::CGameObject*    m_goOwner{};
    Engine::CGameObject*    m_goTarget{};
    CTransform              m_trTitan{};
    CTransform              m_trSensor{};

    DISPLACEMENT            m_tDisplacement{ _float3{} };
    _int                    m_iTargetMask = 0;

    Engine::CEvent<Engine::CGameObject*>        m_OnDetected_Target;

private :
    void OnTriggerEnter(const COLLISION_DESC& tDesc);

public :
    void                    Set_Target(Engine::CGameObject* goTarget);
    void                    Set_TargetMask(_int iMask);
    void                    Clear_Target();
    _bool                   Has_Target() const;
    Engine::CGameObject*    Get_Target() const;

    DISPLACEMENT            Detect_Target(Engine::CGameObject* goTarget);
    DISPLACEMENT            Get_TargetDisplacement() const;

    template <typename T>
    ListenerID Subscribe_OnDetectedTarget(void(T::* func)(Engine::CGameObject*), T* pInstance)
    {
        return m_OnDetected_Target.Add_Listener(func, pInstance);
    }
};

NS_END
