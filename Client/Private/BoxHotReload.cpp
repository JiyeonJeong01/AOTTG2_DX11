#include "BoxHotReload.h"

NS_BEGIN(Client)

void CBoxHotReload::Awake(void* pCtx)
{
}

void CBoxHotReload::Start(void* pCtx)
{
}

void CBoxHotReload::Priority_Update(void* pCtx, _float fDT)
{
}

void CBoxHotReload::Update(void* pCtx, _float fDT)
{
    CGameObject* go = SYS_GAMEOBJECT.Get_Wrapper(m_hObject);
    if (go && go->Is_Valid())
    {
        _fvector deltaPos = { -2.f * fDT, 0.f, 0.f, 0.f };
        CTransform tr = go->Get_Component<CTransform>();
        tr.Translate(deltaPos);
    }
}

void CBoxHotReload::Late_Update(void* pCtx, _float fDT)
{
}

NS_END;
