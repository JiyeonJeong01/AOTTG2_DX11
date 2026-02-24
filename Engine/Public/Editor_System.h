#pragma once
#include "Base.h"

#include "BuiltIn_GUID.h"

NS_BEGIN(Engine)

class ENGINE_DLL CEditor_System final
{
    DECLARE_SINGLETON(CEditor_System)

public:
    HRESULT Initialize(const std::filesystem::path& assetRoot);

public:
    ASSET_GUID Ensure_DefaultScene();
    ASSET_GUID Create_NewScene_Asset(std::filesystem::path* outPath = nullptr);

    void        On_SceneChanged(EVENT_DATA& event);

    void        Play();
    void        Pause();
    void        Step(_float fDT);

private : 
    std::filesystem::path   m_pathAsset{};
    class CScene*           m_pCurScene{};
};



NS_END

