#pragma once

#include "Client_Define.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
class CGameObject;
NS_END

NS_BEGIN(Client)

class CMainApp final : public CBase
{
private:
    CMainApp();
    virtual ~CMainApp() = default;

public:
    HRESULT Initialize();
    void Update(_float fDT);
    void Late_Update(_float fDT);
    void Fixed_Update(_float fDT);

    HRESULT Render();

private:
    CGameInstance* m_pGameInstance{ };
    ID3D11Device* m_pDevice{ };
    ID3D11DeviceContext* m_pContext{ };

    list<CGameObject*> m_GameObjects;

public:
    static CMainApp* Create();
    virtual void Free() override;
};

NS_END
