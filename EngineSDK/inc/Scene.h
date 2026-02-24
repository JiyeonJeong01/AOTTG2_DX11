#pragma once
#include "Engine_Define.h"
#include "Identity.h"

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

public :
    const ASSET_GUID&   Get_GUID() const;
    void                Set_GUID(const ASSET_GUID& tGUID);

    SCENE_STATE         Get_State() const;
    void                Set_State(SCENE_STATE eState);

protected:
    ID3D11Device*           m_pDevice = { nullptr };
    ID3D11DeviceContext*    m_pContext = { nullptr };

    ASSET_GUID              m_tGUID{ };
    SCENE_STATE             m_eState{ };

public :
    static std::unique_ptr<CScene> Create();
};

NS_END
