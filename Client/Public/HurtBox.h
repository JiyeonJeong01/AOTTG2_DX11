#pragma once
#include "Client_Define.h"
#include "Script.h"

NS_BEGIN(Client)

class CHurtBox : public IScript
{
public:
    CHurtBox();
    ~CHurtBox();

    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    CGameObject*            m_pOwner{};
    CEvent<const HIT_INFO&> m_OnHit;

public :
    void Try_ApplyHit(const HIT_INFO& tHitBox);

    template <typename T>
    ListenerID Subscribe_OnHit(void(T::* func)(const HIT_INFO&), T* pInstance)
    {
        return m_OnHit.Add_Listener(func, pInstance);
    }

private :
    SCRIPT_OBJECT_REF   m_refOwner;

SCRIPT_FIELDS_BEGIN(CHurtBox)
    SCRIPT_FIELD_OBJECT_REF(m_refOwner)
SCRIPT_FIELDS_END(CHurtBox)
};

NS_END;
