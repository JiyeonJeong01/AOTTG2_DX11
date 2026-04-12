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
    CGameObject*            m_goHurtBox{};

    CEvent<const HIT_INFO&, const string&> m_OnHurt;

public :
    void Try_ApplyHit(const HIT_INFO& tHitBox);
    CGameObject*    Get_Owner() const;
    CGameObject*    Get_HurtBoxObject() const;

    template <typename T>
    ListenerID Subscribe_OnHurt(void(T::* func)(const HIT_INFO&, const string&), T* pInstance)
    {
        return m_OnHurt.Add_Listener(func, pInstance);
    }

private :
    SCRIPT_OBJECT_REF   m_refOwner;

SCRIPT_FIELDS_BEGIN(CHurtBox)
    SCRIPT_FIELD_OBJECT_REF(m_refOwner)
SCRIPT_FIELDS_END(CHurtBox)
};

NS_END;
