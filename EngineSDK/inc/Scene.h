#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

class ENGINE_DLL CScene : public LABEL
{
public :
    CScene();
    CScene(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CScene();

public:
    virtual HRESULT Initialize();
    virtual void    Update(_float fTimeDelta);
    virtual HRESULT Render();

protected:
    ID3D11Device*           m_pDevice = { nullptr };
    ID3D11DeviceContext*    m_pContext = { nullptr };

public :
    static std::unique_ptr<CScene> Create();
};

NS_END
