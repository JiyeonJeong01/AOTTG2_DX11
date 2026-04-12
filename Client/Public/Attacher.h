#pragma once
#include "Client_Define.h"

NS_BEGIN(Client)

class CAttacher final : public IScript
{
public:
    CAttacher();
    ~CAttacher() override;

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;
    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

    CGameObject*    Get_AttachObject() const;
    void            Stop_Attach();
    void            Start_Attach();

private:
    struct ATTACH_NODE
    {
        SCRIPT_OBJECT_REF   refObject{};
        _char               szBoneName[64] = { 0, };
        _float3             vOffset = { 0.f, 0.f, 0.f };

        ANIMATOR_DATA*      pAnimData = nullptr;
        _uint               iBoneIndex = 0;

        CTransform          trChild{};
    };
    _bool                   m_bCanAttach = true;

private:
    void Register_Attach(ATTACH_NODE& tNode);
    void Sync_Attach(ATTACH_NODE& tNode);

private:
    Engine::CGameObject* m_pOwner = nullptr;
    Engine::CGameObject* m_goAttach = nullptr;
    CTransform              m_trOwner{};

public:
    SCRIPT_FIELDS_BEGIN(CAttacher)
        SCRIPT_FIELD_OBJECT_REF(m_tAttachObject.refObject)
        SCRIPT_FIELD_CHAR(m_tAttachObject.szBoneName)
        SCRIPT_FIELD_FLOAT3(m_tAttachObject.vOffset)
    SCRIPT_FIELDS_END(CAttacher)

private:
    ATTACH_NODE m_tAttachObject{};


public:
    static std::shared_ptr<CAttacher> Create();
};

NS_END
