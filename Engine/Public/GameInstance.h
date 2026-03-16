#pragma once
#include "Engine_Define.h"
#include "Component_Struct.h"
#include "Identity.h"
#include "Component_Struct.h"
#include "Component_System.h"

#pragma region FOWARD DECLARATION
NS_BEGIN(Engine)
class CGameObject;
class CPhysics_Processor;
class CLine;
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
    HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    HRESULT SetUp_Game();

public: /* ----------- Scene ----------- */
    HRESULT Change_Scene(const std::string& strScene);

public: /* -------- GameObject --------- */
    CGameObject* Find_GameObject(OBJECT_HANDLE hObj);
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

public : /* ---------- Game ---------- */
    const _float3&     Cam_Position();


public :/* ---------- Built-in ---------- */
    unique_ptr<CLine>   Load_LineMesh(_uint iNumPoint, _float fThickness);
    void                Test_LineRibbonMesh();


private :
    ID3D11Device*               m_pDevice = nullptr;
    ID3D11DeviceContext*        m_pContext = nullptr;

    GameConfig::GAME_CONFIG     m_tGameConfig{};

    CPhysics_Processor*         m_pPhysics{};


private : /* ----------- Scene ----------- */
    _bool Read_GameConfig();
    _bool Resolve_SceneGUID(const std::string& strScene, ASSET_GUID& outGUID);
};


NS_END
