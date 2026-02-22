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
    void            Destroy_Object(CGameObject* pObj);

    void            Set_Layer(CGameObject* pObj, Layer::LAYER_ID iNewLayer);
    const std::vector<CGameObject*>& Get_LayerObjects(Layer::LAYER_ID iLayer) const;
    void            Gather_By_Mask(Layer::LAYER_MASK mask, std::vector<CGameObject*>& outObjects) const;
    void            Get_Roots(std::vector<CGameObject*>& outRoots);

    void                    Set_UUID(CGameObject* pObj, const INSTANCE_UUID& tUUID);
    const INSTANCE_UUID&    Get_UUID(CGameObject* pObj);

    GAMEOBJECT_DATA&    Access_Data_Raw(OBJECT_HANDLE hObj);
    CGameObject*        Get_Wrapper(OBJECT_HANDLE hObj);
    _bool               Is_Valid_Handle(OBJECT_HANDLE hObj) const;

    HRESULT             Build_SceneSpecs(std::vector<SCENE_OBJECT_SPEC>& outSpecs);

    /* --- Helpers ---*/
public :
    CLayerHelper& Layers() const;
    std::unique_ptr<CLayerHelper>               m_pLayerHelper{};

private:
    CGameObject* Create_Object(Layer::LAYER_ID iLayer, const string& strName, CGameObject* pParent, const INSTANCE_UUID& tUUID);
private :
    using Objects = std::vector<CGameObject*>;

    uint32_t                                    m_iLayerCount = Layer::MAX_LAYERS;
    std::array<Objects, Layer::MAX_LAYERS>      m_layerBuckets{};
    std::vector<GAMEOBJECT_DATA>                m_dataPool;
    std::vector<std::unique_ptr<CGameObject>>   m_wrapperPool;  /* Exclusive ownership */
    std::queue<uint32_t>                        m_freeIndices;
    std::vector<uint32_t>                       m_pendingDestroys;


private:
    void Remove_From_LayerBucket(CGameObject* pObj);
    void Add_To_LayerBucket(CGameObject* pObj, Layer::LAYER_ID layer = Layer::DEFAULT_LAYER);
    void Flush_PendingDestroy();
};


NS_END
