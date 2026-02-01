#include "GameObject_System.h"
#include "GameObject.h"
#include "Engine_Log.h"

NS_BEGIN(Engine)

IMPLEMENT_SINGLETON(CGameObject_System)

CGameObject_System::CGameObject_System()
{
}

HRESULT CGameObject_System::Initialize(uint32_t iMaxLayers)
{
    if (iMaxLayers <= Layer::DEFAULT_LAYER || iMaxLayers > Layer::MAX_LAYERS)
    {
        _DEBUG_ERROR_BREAK("CGameObject_System Initialize failed: invalid layer count.");
        return E_FAIL;
    }

    m_iLayerCount = iMaxLayers;

    for (uint32_t i = 0; i < Layer::MAX_LAYERS; ++i)
        m_layerBuckets[i].clear();

    return S_OK;
}

HRESULT CGameObject_System::Create_Object(CGameObject* pNewObj, Layer::LAYER_ID iLayer/* = Layer::DEFAULT_LAYER*/)
{
    if (iLayer == Layer::INVALID_LAYER || iLayer >= m_iLayerCount)
    {
        _DEBUG_WARN("Invalid layer; set to 0");
        iLayer = 0;
    }

    if (!pNewObj)
    {
        _DEBUG_ERROR_BREAK("Create_Object failed: CGameObject::Create() returned null.");
        return E_FAIL;
    }

    /* GAMEOBJECT_META init + Layerbucket add */
    Add_To_LayerBucket(pNewObj, iLayer);

    return S_OK;
}

void CGameObject_System::Destroy_Object(CGameObject* pObj)
{
    if (!pObj)
    {
        _DEBUG_WARN("Destroy_Object failed : CGameObject is nullptr");
        return;
    }

    Remove_From_LayerBucket(pObj);

    m_pendingDestroy.push_back(pObj);
}

void CGameObject_System::Set_Layer(CGameObject* pObj, Layer::LAYER_ID iNewLayer)
{
    if (!pObj)
    {
        _DEBUG_ERROR_BREAK("Set_Layer failed : GameObject is nullptr");
        return;
    }

    if (iNewLayer >= m_iLayerCount || iNewLayer == Layer::INVALID_LAYER)
    {
        _DEBUG_WARN("Invalid layer index; set to 0.");
        iNewLayer = 0;
    }

    auto& tMeta = pObj->Access_Meta();
    if (tMeta.layer == iNewLayer)
        return;

    /* Remove from the previous layer and insert into the new layer. */
    Remove_From_LayerBucket(pObj);
    Add_To_LayerBucket(pObj, iNewLayer);
}

const std::vector<CGameObject*>& CGameObject_System::Get_LayerObjects(Layer::LAYER_ID iLayer) const
{
    static const std::vector<CGameObject*> s_Empty;

    if (iLayer >= m_iLayerCount || iLayer == Layer::INVALID_LAYER)
    {
        _DEBUG_WARN("Invalid layer index; set to 0.");
        return s_Empty;
    }

    return m_layerBuckets[iLayer];
}

void CGameObject_System::Gather_By_Mask(Layer::LAYER_MASK mask, std::vector<CGameObject*>& outObjects) const
{
    outObjects.clear();

    /* Reserve to gather GameObjects */
    size_t iTotal = 0;

    for (uint32_t iLayer = 0; iLayer < m_iLayerCount; ++iLayer)
    {
        if ((mask & Layer::To_Bit(SCAST(Layer::LAYER_ID, iLayer))) == 0)
            continue;
        iTotal += m_layerBuckets[iLayer].size();
    }

    outObjects.reserve(iTotal);

    /* Gather GameObjects */
    for (uint32_t iLayer = 0; iLayer < m_iLayerCount; ++iLayer)
    {
        if ((mask & Layer::To_Bit(SCAST(Layer::LAYER_ID, iLayer))) == 0)
            continue;

        const auto& layerBucket = m_layerBuckets[iLayer];
        outObjects.insert(outObjects.end(), layerBucket.begin(), layerBucket.end());
    }
}

void CGameObject_System::Get_Roots(std::vector<CGameObject*>& outRoots) const
{
    outRoots.clear();

    for (uint32_t iLayer = 0; iLayer < m_iLayerCount; ++iLayer)
    {
        const auto& layerBucket = m_layerBuckets[iLayer];
        for (CGameObject* pObj : layerBucket)
        {
            if (!pObj)
                continue;
            if (!pObj->Get_Parent())
                outRoots.push_back(pObj);
        }
    }
}

void CGameObject_System::Remove_From_LayerBucket(CGameObject* pObj)
{
    if (!pObj)
    {
        _DEBUG_WARN("Remove_From_LayerBucket: pObj is nullptr");
        return;
    }

    auto& tMeta = pObj->Access_Meta();

    if (tMeta.layer == Layer::INVALID_LAYER)
    {
        _DEBUG_WARN("Invalid meta");
        return;
    }

    if (tMeta.layer >= m_iLayerCount)
    {
        _DEBUG_WARN("Remove_From_LayerBucket: meta.layer out of range. Force reset.");
        tMeta.layer = Layer::INVALID_LAYER;
        tMeta.iIndexInLayer = 0;
        return;
    }

    auto& bucket = m_layerBuckets[tMeta.layer];

    if (bucket.empty())
    {
        _DEBUG_WARN("Remove_From_LayerBucket: bucket empty but meta says registered.");
        tMeta.layer = Layer::INVALID_LAYER;
        tMeta.iIndexInLayer = 0;
        return;
    }

    const uint32_t iRemoveIndex = tMeta.iIndexInLayer;
    const uint32_t iLastIndex = SCAST(uint32_t, bucket.size() - 1);

    if (iRemoveIndex > iLastIndex)
    {
        _DEBUG_ERROR_BREAK("Meta is in a corrupted state.");
        tMeta.layer = Layer::INVALID_LAYER;
        tMeta.iIndexInLayer = 0;
        return;
    }

    /* Swap-pop*/
    if (iRemoveIndex != iLastIndex)
    {
        CGameObject* pMoved = bucket[iLastIndex];
        bucket[iRemoveIndex] = pMoved;

        if (pMoved)
        {
            auto& tMovedMeta = pMoved->Access_Meta();

            tMovedMeta.layer = tMeta.layer;
            tMovedMeta.iIndexInLayer = iRemoveIndex;
        }
        else
        {
            _DEBUG_WARN("Remove_From_LayerBucket: moved object is nullptr.");
        }
    }

    bucket.pop_back();

    /* Reset pObj meta data to an unregistered state. */
    tMeta.layer = Layer::INVALID_LAYER;
    tMeta.iIndexInLayer = 0;
}

void CGameObject_System::Add_To_LayerBucket(CGameObject* pObj, Layer::LAYER_ID layer /* = Layer::DEFAULT_LAYER*/)
{
    if (!pObj)
    {
        _DEBUG_WARN("Add_To_LayerBucket: pObj is nullptr");
        return;
    }

    if (layer == Layer::INVALID_LAYER || layer >= m_iLayerCount)
    {
        _DEBUG_WARN("Add_To_LayerBucket: invalid layer index; set to 0.");
        layer = 0;
    }

    auto& tMeta = pObj->Access_Meta();

    /* Prevent duplicate registration. In the Set_Layer flow, Remove is called before Add, so this should not normally trigger. */ 
    if (tMeta.layer != Layer::INVALID_LAYER)
    {
        _DEBUG_WARN("Add_To_LayerBucket: object already registered. Removing from old layer first.");
        Remove_From_LayerBucket(pObj);
    }

    auto& bucket = m_layerBuckets[layer];
    tMeta.layer = layer;
    tMeta.iIndexInLayer = SCAST(uint32_t, bucket.size());
    bucket.push_back(pObj);

    _DEBUG_INFO("layer : %d, Index in Layer : %d", SCAST(int, tMeta.layer), tMeta.iIndexInLayer);
}

void CGameObject_System::Flush_PendingDestroy()
{
    for (CGameObject* pObj : m_pendingDestroy)
    {
        if (!pObj)
            continue;

        /* TODO : Proper handling is needed for the destory */

        Safe_Release(pObj);
    }
}

CGameObject_System* CGameObject_System::Create(uint32_t iMaxLayers)
{
    CGameObject_System* pInstance = new CGameObject_System();
    if (FAILED(pInstance->Initialize(iMaxLayers)))
    {
        Safe_Release(pInstance);
        _DEBUG_ERROR_BREAK("Create CGameObjectSystem faild");
    }
    return pInstance;
}

void CGameObject_System::Free()
{
    CBase::Free();
}

NS_END
