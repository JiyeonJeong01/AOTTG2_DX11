#pragma once

#include "Client_Define.h"
#include "Base.h"

/* ================== TEST ================== */

#include "Event.h"

/* ========================================== */



NS_BEGIN(Engine)
class CGameInstance;
class CGameObject;
NS_END

NS_BEGIN(Client)

class CMainApp final : public CBase
{
private:
    CMainApp();
    ~CMainApp() override = default;

public:
    HRESULT Initialize(const ENGINE_DESC& EngineDesc);
    void Update(_float fDT);
    void Late_Update(_float fDT);
    void Fixed_Update(_float fDT);

    HRESULT Begin_Render();
    HRESULT Render();
    HRESULT End_Render();

private:
    CGameInstance* m_pGameInstance{ };
    ID3D11Device* m_pDevice{ };
    ID3D11DeviceContext* m_pContext{ };

    /* ================== TEST ================== */
public :
    Engine::CEvent<> m_voidEvent;
    Engine::CEvent<_int> m_intEvent;
    Engine::CEvent<_int, _float> m_intFloatEvent;
    class Tester* m_pTester{};


private :

    list<CGameObject*> m_GameObjects;

    /* ========================================== */

public:
    static CMainApp* Create(const ENGINE_DESC& EngineDesc);
    virtual void Free() override;
};

NS_END
