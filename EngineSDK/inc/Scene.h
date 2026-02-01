#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CScene : public CBase, public LABEL
{
protected:
    CScene(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CScene() = default;

public:
    virtual HRESULT Initialize();
    virtual void Update(_float fTimeDelta);
    virtual HRESULT Render();

protected:
    ID3D11Device*           m_pDevice = { nullptr };
    ID3D11DeviceContext*    m_pContext = { nullptr };

public:
    virtual void Free() override;
};

NS_END
