#pragma once

#include "Client_Define.h"

NS_BEGIN(Client)
class CScout;
class CScout_Controller : public IScript
{
private:
    typedef struct tagStagingScout
    {
        CGameObject* pObject = nullptr;
        CTransform      tr{};
        CMeshRenderer   meshRenderer{};
        CAnimator       anim{};
    } STAGING_SCOUT;

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

public:
    void Ready_Opening();
    void Finish_Opening();

    void Enable_Object(_bool bEnable);
    void Play_Salute_Animation();

private:
    void Cache_Scouts();

private:
    static constexpr size_t NUM_SCOUT = 10;

    SCRIPT_OBJECT_REF       m_refScout1{};
    SCRIPT_OBJECT_REF       m_refScout2{};

    CGameObject*            m_goScout1{};
    CGameObject*            m_goScout2{};

    CScout* m_scScout1{};
    CScout* m_scScout2{};
    

    SCRIPT_OBJECT_REF       m_refScouts[NUM_SCOUT]{};
    std::vector<STAGING_SCOUT>   m_vecStagingScouts{};

    SCRIPT_FIELDS_BEGIN(CScout_Controller)
        SCRIPT_FIELD_OBJECT_REF(m_refScouts[0])
        SCRIPT_FIELD_OBJECT_REF(m_refScouts[1])
        SCRIPT_FIELD_OBJECT_REF(m_refScouts[2])
        SCRIPT_FIELD_OBJECT_REF(m_refScouts[3])
        SCRIPT_FIELD_OBJECT_REF(m_refScouts[4])
        SCRIPT_FIELD_OBJECT_REF(m_refScouts[5])
        SCRIPT_FIELD_OBJECT_REF(m_refScouts[6])
        SCRIPT_FIELD_OBJECT_REF(m_refScouts[7])
        SCRIPT_FIELD_OBJECT_REF(m_refScouts[8])
        SCRIPT_FIELD_OBJECT_REF(m_refScouts[9])

        SCRIPT_FIELD_OBJECT_REF(m_refScout1)
        SCRIPT_FIELD_OBJECT_REF(m_refScout2)
        SCRIPT_FIELDS_END(CScout_Controller)
};

NS_END
