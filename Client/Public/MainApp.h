#pragma once

#include "Client_Define.h"

/* ================== TEST ================== */

#include "Event.h"
#include "Component_Spec.h"

/* ========================================== */

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class CMainApp final
{
public:
    CMainApp();
    ~CMainApp();

public:
    HRESULT Initialize(const ENGINE_DESC& EngineDesc);
    void Update(_float fDT);
    void Late_Update(_float fDT);
    void Fixed_Update(_float fDT);

    HRESULT Begin_Render();
    HRESULT Render();
    HRESULT End_Render();

private:
    ID3D11Device* m_pDevice{ };
    ID3D11DeviceContext* m_pContext{ };

    /* ================== TEST ================== */
public :
    Engine::CEvent<> m_voidEvent;
    Engine::CEvent<_int> m_intEvent;
    Engine::CEvent<_int, _float> m_intFloatEvent;
    class Tester* m_pTester{};

private :
    _float4		vClearColor = { 0.18f, 0.18f, 0.18f, 1.0f };

    /* ========================================== */

public:
    static unique_ptr<CMainApp> Create(const ENGINE_DESC& EngineDesc);
};

NS_END
