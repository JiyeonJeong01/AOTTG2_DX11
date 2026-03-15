#pragma once
#include "Engine_Define.h"
#include "Component_Struct.h"
#include "Identity.h"

#pragma region FOWARD DECLARATION
NS_BEGIN(Engine)
class CGameObject;
class CPhysics_Processor;
typedef struct tagRay RAY;
typedef struct tagRaycastHit RAYCAST_HIT;
typedef struct tagRaycastHits RAYCAST_HITS;
NS_END
#pragma endregion

NS_BEGIN(Engine)

class ENGINE_DLL CGameInstance final
{
    DECLARE_SINGLETON(CGameInstance)

public:
    HRESULT Initialize();
    HRESULT SetUp_Game();

public: /* ----------- Scene ----------- */
    void Change_Scene(const std::string& strScene);

public: /* -------- GameObject --------- */
    CGameObject* Get_GameObject(OBJECT_HANDLE hObj);
    CGameObject* Get_GameObject(COMPONENT_HANDLE hComponent);
    CGameObject* Find_GameObject(const std::string& strName);

public : /* -------- Raycast -------- */
    _bool   Raycast(const POINT& pt, RAY& tRAY, RAYCAST_HIT& tHitInfo);
    _bool   RaycastAll(const POINT& pt, RAY& tRAY, RAYCAST_HITS& tAllHitInfo);
    void    Test_Raycast();

public : /* ---------- Game ---------- */
    _float      Get_DT() const noexcept;
    void        Set_TimeScale();
    void        Pause();
    void        Play();

private :
    GameConfig::GAME_CONFIG     m_tGameConfig{};

    CPhysics_Processor*         m_pPhysics{};


private : /* ----------- Scene ----------- */
    _bool Read_GameConfig();
    _bool Resolve_SceneGUID(const std::string& strScene, ASSET_GUID& outGUID);
};

NS_END
