#pragma once

#include "Client_Define.h"

/* ================== TEST ================== */

#include "Event.h"
#include "Component_Spec.h"
#include "BUTTON_EVENT_DATA.h"
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
    void Update(_float fDT, Engine::APP_MODE eMode);
    void Late_Update(_float fDT);
    void Fixed_Update(_float fDT);

    HRESULT Render();

private:
    ID3D11Device* m_pDevice{ };
    ID3D11DeviceContext* m_pContext{ };

private :
    _float4		vClearColor = { 0.18f, 0.18f, 0.18f, 1.0f };
    

public:
    static unique_ptr<CMainApp> Create(const ENGINE_DESC& EngineDesc);
};

NS_END
