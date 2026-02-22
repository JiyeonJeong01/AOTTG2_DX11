#pragma once

#include "Engine_Define.h"
#include "Spec_Struct.h"

NS_BEGIN(Engine)

class CGameObject;
class CScene;
class CCore_System;

class CScene_Handler final
{
    using SpecFactoryFn = std::unique_ptr<COMPONENT_SPEC_BASE>(*)(COMPONENT_TYPE);

public:
    CScene_Handler();
    ~CScene_Handler();

public:
    HRESULT Change_Scene(_uint iNewSceneIndex, std::unique_ptr<CScene> pNewScene);
    void    Update(_float fTimeDelta);
    HRESULT Render();

private:
    std::unique_ptr<CScene> m_pCurrentScene{};

    _uint			m_iCurrentSceneIndex = 0;

    std::vector<SCENE_OBJECT_SPEC>  m_SceneObjectSpecs;

public:
    _bool   Save_CurrentScene(const std::filesystem::path& path);

    /* SCENE_OBJECT_SPEC -> JSON */
    _bool    Save_SceneFile(const std::vector<SCENE_OBJECT_SPEC>& objects, const std::filesystem::path& path);
    json     Serialize_SceneObjectSpec(const SCENE_OBJECT_SPEC& tSpec);

    /* JSON -> SCENE_OBJECT_SPEC */
    _bool    Load_SceneFile(std::vector<SCENE_OBJECT_SPEC>& outObjects, const std::filesystem::path& path, SpecFactoryFn createSpec);
    _bool    Deserialize_SceneObjectSpec(const json& j, SCENE_OBJECT_SPEC& out, SpecFactoryFn createSpec);

    /* SCENE_OBJECT_SPEC -> CGameObject */
    HRESULT  LoadScene_Runtime(const std::vector<SCENE_OBJECT_SPEC>& tSpecs);
    HRESULT  Apply_Overrides(CGameObject* pObject, const COMPONENT_SPEC_BUNDLE& tBundle);

    static std::unique_ptr<COMPONENT_SPEC_BASE> Create_Spec_By_Type(COMPONENT_TYPE eType);
    static std::unique_ptr<CScene_Handler> Create();
};

NS_END
