#pragma once

#include "Engine_Define.h"

NS_BEGIN(Engine)

class CScene;
class CCore_System;

class CScene_Manager final
{
public:
    CScene_Manager();
    ~CScene_Manager();

public:
    HRESULT Change_Scene(_uint iNewSceneIndex, std::unique_ptr<CScene> pNewScene);
    void    Update(_float fTimeDelta);
    HRESULT Render();

private:
    std::unique_ptr<CScene> m_pCurrentScene{};

    _uint			m_iCurrentSceneIndex = 0;

public:
    static std::unique_ptr<CScene_Manager> Create();
};

NS_END
