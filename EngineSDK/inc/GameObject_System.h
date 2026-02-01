#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CGameObject;

class ENGINE_DLL CGameObject_System final : public CBase
{
    DECLARE_SINGLETON(CGameObject_System)
private:
    CGameObject_System();
    ~CGameObject_System() override = default;

public:
    HRESULT         Initialize(uint32_t iMaxLayers = 32);

    HRESULT         Create_Object(CGameObject* pNewObj, Layer::LAYER_ID iLayer = Layer::DEFAULT_LAYER);
    void            Destroy_Object(CGameObject* pObj);

    void            Set_Layer(CGameObject* pObj, Layer::LAYER_ID iNewLayer);
    const std::vector<CGameObject*>& Get_LayerObjects(Layer::LAYER_ID iLayer) const;
    void            Gather_By_Mask(Layer::LAYER_MASK mask, std::vector<CGameObject*>& outObjects) const;
    void            Get_Roots(std::vector<CGameObject*>& outRoots) const;

private:
    uint32_t        m_iLayerCount = Layer::MAX_LAYERS;
    std::array<std::vector<CGameObject*>, Layer::MAX_LAYERS> m_layerBuckets{};
    std::list<CGameObject*> m_pendingDestroy;

private:
    void Remove_From_LayerBucket(CGameObject* pObj);
    void Add_To_LayerBucket(CGameObject* pObj, Layer::LAYER_ID layer = Layer::DEFAULT_LAYER);
    void Flush_PendingDestroy();

public:
    static CGameObject_System* Create(uint32_t iMaxLayers = 32);
    void Free() override;
};

NS_END
