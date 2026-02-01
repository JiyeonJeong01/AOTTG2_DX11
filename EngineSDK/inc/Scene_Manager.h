#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CScene;
class CCore_System;

class CScene_Manager final : public CBase
{
private:
    CScene_Manager();
    virtual ~CScene_Manager() = default;

public:
    HRESULT Change_Scene(_uint iNewSceneIndex, class CScene* pNewScene);
    void    Update(_float fTimeDelta);
    HRESULT Render();

private:
    CScene*         m_pCurrentScene = nullptr;

    _uint			m_iCurrentSceneIndex = 0;

public:
    static CScene_Manager* Create();
    virtual void Free() override;
};

NS_END
