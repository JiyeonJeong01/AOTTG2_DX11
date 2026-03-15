#pragma once

#include "Engine_Define.h"
#include "Object_Struct.h"
#include "Spec_Struct.h"

NS_BEGIN(Engine)
    class CGameObject;
class CLayerHelper;

class ENGINE_DLL CGameObject_System final
{
    DECLARE_SINGLETON(CGameObject_System)

public:
    HRESULT         Initialize(uint32_t iMaxLayers = 32, uint32_t iPoolSize = 2048);
    CGameObject*    Create_GameObject(Layer::LAYER_ID iLayer = Layer::DEFAULT_LAYER,
                                    const string& strName = "GameObject",
                                    CGameObject* pParent = nullptr,
                                    const INSTANCE_UUID& tUUID = INSTANCE_UUID{});
    CGameObject*    Create_GameObjectUI(Layer::LAYER_ID iLayer = Layer::UI_LAYER,
                                    const string& strName = "UIObject",
                                    CGameObject* pParent = nullptr,
                                    const INSTANCE_UUID& tUUID = INSTANCE_UUID{});
    CGameObject*    Instantiate(const string& strProto,
                                    Layer::LAYER_ID iLayer = Layer::DEFAULT_LAYER,
                                    const string& strName = "GameObject_Clone",
                                    CGameObject* pParent = nullptr);
    CGameObject*    Instantiate(const ASSET_GUID& tGUID,
                                    Layer::LAYER_ID iLayer = Layer::DEFAULT_LAYER,
                                    const string& strName = "GameObject_Clone",
                                    CGameObject* pParent = nullptr);

    void            Destroy_Object(CGameObject* pObj);
    void            Destroy_All_SceneObjects();
    void            Flush_PendingDestroy();

    void            Set_Enable(CGameObject* pObj, _bool bEnable);

public :
    void            Set_Layer(CGameObject* pObj, Layer::LAYER_ID iNewLayer);
    const std::vector<CGameObject*>& Get_LayerObjects(Layer::LAYER_ID iLayer) const;
    void            Gather_By_Mask(Layer::LAYER_MASK mask, std::vector<CGameObject*>& outObjects) const;
    void            Get_Roots(std::vector<CGameObject*>& outRoots);

    void                    Set_UUID(CGameObject* pObj, const INSTANCE_UUID& tUUID);
    const INSTANCE_UUID&    Get_UUID(CGameObject* pObj);

    GAMEOBJECT_DATA&    Access_Data_Raw(OBJECT_HANDLE hObj);
    CGameObject*        Get_Wrapper(OBJECT_HANDLE hObj);
    CGameObject*        Find_GameObject(const std::string& strLabel);
    _bool               Is_Valid_Handle(OBJECT_HANDLE hObj) const;

    HRESULT             Build_SceneSpecs(std::vector<SCENE_OBJECT_SPEC>& outSpecs);

public:
    OBJECT_HANDLE Find_Handle_By_UUID(const INSTANCE_UUID& tUUID) const;
    void Register_UUID_Handle(const INSTANCE_UUID& tUUID, OBJECT_HANDLE hObject);
    void Unregister_UUID_Handle(const INSTANCE_UUID& tUUID);

private:

    /* --- Helpers ---*/
public :
    CLayerHelper& Layers() const;
    std::unique_ptr<CLayerHelper>               m_pLayerHelper{};

private:
    CGameObject* Create_Object_Inner(Layer::LAYER_ID iLayer, const string& strName, CGameObject* pParent, const INSTANCE_UUID& tUUID);
private :
    using Objects = std::vector<CGameObject*>;

    uint32_t                                    m_iLayerCount = Layer::MAX_LAYERS;
    std::array<Objects, Layer::MAX_LAYERS>      m_layerBuckets{};
    std::vector<GAMEOBJECT_DATA>                m_dataPool;
    std::vector<std::unique_ptr<CGameObject>>   m_wrapperPool;  /* Exclusive ownership */
    std::queue<uint32_t>                        m_freeIndices;
    std::vector<uint32_t>                       m_pendingDestroys;

    CGameObject*                                m_pCanvas{};

    std::unordered_map<INSTANCE_UUID, OBJECT_HANDLE, INSTANCE_UUID_HASHER> m_mapUUIDToHandle;


private:
    void Remove_From_LayerBucket(CGameObject* pObj);
    void Add_To_LayerBucket(CGameObject* pObj, Layer::LAYER_ID layer = Layer::DEFAULT_LAYER);
};


NS_END
