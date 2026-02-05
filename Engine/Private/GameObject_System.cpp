#include "GameObject_System.h"
#include "GameObject.h"
#include "Engine_Log.h"

NS_BEGIN(Engine)

IMPLEMENT_SINGLETON(CGameObject_System)

CGameObject_System::CGameObject_System()
{
}

HRESULT CGameObject_System::Initialize(uint32_t iMaxLayers, uint32_t iPoolSize)
{
    if (iMaxLayers <= Layer::DEFAULT_LAYER || iMaxLayers > Layer::MAX_LAYERS)
    {
        _DEBUG_ERROR_BREAK("CGameObject_System Initialize failed: invalid layer count.");
        return E_FAIL;
    }

    if (iPoolSize < 2)
    {
        _DEBUG_ERROR_BREAK("CGameObject_System Initialize failed: pool size too small.");
        return E_FAIL;
    }

    m_iLayerCount = iMaxLayers;

    for (uint32_t i = 0; i < Layer::MAX_LAYERS; ++i)
        m_layerBuckets[i].clear();

    m_dataPool.clear();
    m_wrapperPool.clear();

    m_dataPool.resize(iPoolSize);
    m_wrapperPool.resize(iPoolSize, nullptr);

    while (!m_freeIndices.empty())
        m_freeIndices.pop();

    /* m_wrapperPool[0] is for DummyData */
    m_wrapperPool[0] = nullptr;

    for (uint32_t i = 1; i < iPoolSize; ++i)
    {
        CGameObject* pWrapper = new CGameObject(i);
        m_wrapperPool[i] = pWrapper;

        m_freeIndices.push(i);
        m_dataPool[i].Reset();
    }

    return S_OK;
}

CGameObject* CGameObject_System::Create_Object(Layer::LAYER_ID iLayer, const string& strName)
{
    if (m_freeIndices.empty())
    {
        _DEBUG_ERROR_BREAK("GameObject Pool is Full!");
        return nullptr;
    }

    if (iLayer == Layer::INVALID_LAYER || iLayer >= m_iLayerCount)
    {
        _DEBUG_WARN("Invalid layer; set to DEFAULT_LAYER.");
        iLayer = Layer::DEFAULT_LAYER;
    }

    /* Allocate index */
    const uint32_t idx = m_freeIndices.front();
    m_freeIndices.pop();

    GAMEOBJECT_DATA& data = m_dataPool[idx];
    CGameObject* pWrapper = m_wrapperPool[idx];

    if (!pWrapper)
    {
        _DEBUG_ERROR_BREAK("Create_Object failed: wrapper is null.");
        m_freeIndices.push(idx); /* Return allocated index */
        return nullptr;
    }

    /* Reset data */
    data.Reset();
    data.iVersion++;
    data.bActive = true;

    pWrapper->m_hSelf.iIndex = idx;
    pWrapper->m_hSelf.iVersion = data.iVersion;

    pWrapper->Set_Label(strName);

    Add_To_LayerBucket(pWrapper, iLayer);

    return pWrapper;
}

void CGameObject_System::Destroy_Object(CGameObject* pObj)
{
    if (!pObj || !pObj->IsValid())
    {
        _DEBUG_WARN("Destroy_Object failed : Invalid Object");
        return;
    }

    Remove_From_LayerBucket(pObj);

    /* TODO : ======================================================== */
    /* TODO : =============== ADD PENDING DESTROY LOGIC ============== */
    /* TODO : ======================================================== */

    const GAMEOBJECT_HANDLE handle = pObj->Get_Handle();
    if (handle.iIndex == 0 /* dummy */ || handle.iIndex >= m_dataPool.size())
        return;

    GAMEOBJECT_DATA& data = m_dataPool[handle.iIndex];

    /* Clean up the relationship with the parent. */
    if (data.hParent.IsValid())
    {
        CGameObject* pParent = Get_Wrapper(data.hParent);
        if (pParent && pParent->IsValid())
        {
            pObj->Set_Parent(nullptr);
        }
        else
        {
            data.hParent = {};
        }
    }

    /* Clean up the relationship with the children */
    {
        auto childrenCopy = data.hChildren;
        for (const auto& hChild : childrenCopy)
        {
            CGameObject* pChild = Get_Wrapper(hChild);
            if (pChild && pChild->IsValid())
            {
                pChild->Set_Parent(nullptr);
            }
        }
    }

    data.bActive = false;
    data.Reset();

    m_freeIndices.push(handle.iIndex);
}

void CGameObject_System::Set_Layer(CGameObject* pObj, Layer::LAYER_ID iNewLayer)
{
    if (!pObj || !pObj->IsValid())
    {
        _DEBUG_ERROR_BREAK("Set_Layer failed : Invalid Object");
        return;
    }

    if (iNewLayer >= m_iLayerCount || iNewLayer == Layer::INVALID_LAYER)
    {
        _DEBUG_WARN("Invalid layer index; set to DEFAULT_LAYER.");
        iNewLayer = Layer::DEFAULT_LAYER;
    }

    GAMEOBJECT_DATA& data = Access_Data_Raw(pObj->Get_Handle());
    if (data.layer == iNewLayer)
        return;

    Remove_From_LayerBucket(pObj);
    Add_To_LayerBucket(pObj, iNewLayer);
}

const std::vector<CGameObject*>& CGameObject_System::Get_LayerObjects(Layer::LAYER_ID iLayer) const
{
    static const std::vector<CGameObject*> s_Empty;

    if (iLayer >= m_iLayerCount || iLayer == Layer::INVALID_LAYER)
    {
        _DEBUG_WARN("Invalid layer index; returning empty.");
        return s_Empty;
    }

    return m_layerBuckets[iLayer];
}

void CGameObject_System::Gather_By_Mask(Layer::LAYER_MASK mask, std::vector<CGameObject*>& outObjects) const
{
    outObjects.clear();

    size_t total = 0;
    for (uint32_t iLayer = 0; iLayer < m_iLayerCount; ++iLayer)
    {
        if ((mask & Layer::To_Bit(SCAST(Layer::LAYER_ID, iLayer))) == 0)
            continue;

        total += m_layerBuckets[iLayer].size();
    }

    outObjects.reserve(total);

    for (uint32_t iLayer = 0; iLayer < m_iLayerCount; ++iLayer)
    {
        if ((mask & Layer::To_Bit(SCAST(Layer::LAYER_ID, iLayer))) == 0)
            continue;

        const auto& bucket = m_layerBuckets[iLayer];
        for (CGameObject* pObj : bucket)
        {
            if (pObj && pObj->IsValid())
                outObjects.push_back(pObj);
        }
    }
}

void CGameObject_System::Get_Roots(std::vector<CGameObject*>& outRoots)
{
    outRoots.clear();

    for (uint32_t iLayer = 0; iLayer < m_iLayerCount; ++iLayer)
    {
        const auto& bucket = m_layerBuckets[iLayer];
        for (CGameObject* pObj : bucket)
        {
            if (!pObj || !pObj->IsValid())
                continue;

            if (!Access_Data_Raw(pObj->Get_Handle()).hParent.IsValid())
                outRoots.push_back(pObj);
        }
    }
}

void CGameObject_System::Remove_From_LayerBucket(CGameObject* pObj)
{
    if (!pObj || !pObj->IsValid())
        return;

    GAMEOBJECT_DATA& tData = Access_Data_Raw(pObj->Get_Handle());

    const uint32_t iLayer = tData.layer;
    if (iLayer == Layer::INVALID_LAYER || iLayer >= m_iLayerCount)
    {
        _DEBUG_ERROR_BREAK("Invalid layer access.");
        return;
    }

    auto& bucket = m_layerBuckets[iLayer];
    if (bucket.empty())
    {
        _DEBUG_ERROR_BREAK("Invalid layer access : this layer is empty.");
        tData.layer = Layer::INVALID_LAYER;
        tData.iIndexInLayer = 0;
        return;
    }

    const uint32_t iLastIdx = (uint32_t)bucket.size() - 1;

    uint32_t iRemoveIdx = tData.iIndexInLayer;

    /* If metadata index is invalid, recover real index defensively. */
    if (iRemoveIdx > iLastIdx)
    {
        _DEBUG_ERROR_BREAK("Corrupted layer index; fall back to a defensive linear search.");

        auto it = std::find(bucket.begin(), bucket.end(), pObj);
        if (it == bucket.end())
        {
            /* Not found in bucket; just clear layer data. */
            tData.layer = Layer::INVALID_LAYER;
            tData.iIndexInLayer = 0;
            return;
        }

        iRemoveIdx = (uint32_t)std::distance(bucket.begin(), it);
    }

    /* Swap-pop removal. */
    if (iRemoveIdx != iLastIdx)
    {
        CGameObject* pMoved = bucket[iLastIdx];
        bucket[iRemoveIdx] = pMoved;

        if (pMoved && pMoved->IsValid())
            Access_Data_Raw(pMoved->Get_Handle()).iIndexInLayer = iRemoveIdx;
    }

    bucket.pop_back();

    /* Cleanup for the object being removed. */
    tData.layer = Layer::INVALID_LAYER;
    tData.iIndexInLayer = 0;
}


void CGameObject_System::Add_To_LayerBucket(CGameObject* pObj, Layer::LAYER_ID layer)
{
    if (!pObj || !pObj->IsValid())
        return;

    if (layer == Layer::INVALID_LAYER || layer >= m_iLayerCount)
        layer = Layer::DEFAULT_LAYER;

    const GAMEOBJECT_HANDLE hObj = pObj->Get_Handle();
    GAMEOBJECT_DATA& data = Access_Data_Raw(hObj);

    /* Remove this object from its current layer bucket. */
    if (data.layer != Layer::INVALID_LAYER)
        Remove_From_LayerBucket(pObj);

    auto& bucket = m_layerBuckets[layer];

    data.layer = layer;
    data.iIndexInLayer = (uint32_t)bucket.size();

    bucket.push_back(pObj);
}

void CGameObject_System::Flush_PendingDestroy()
{
    // TODO ============================================================
    // TODO implement after system architecture comfirmed
    // TODO ============================================================
}

GAMEOBJECT_DATA& CGameObject_System::Access_Data_Raw(GAMEOBJECT_HANDLE hObj)
{
    if (hObj.iIndex == 0 || hObj.iIndex >= m_dataPool.size())
    {
        _DEBUG_ERROR_BREAK("Access_Data_Raw failed: invalid index.");
        return m_dataPool[0]; // 0번 더미
    }
    return m_dataPool[hObj.iIndex];
}

CGameObject* CGameObject_System::Get_Wrapper(GAMEOBJECT_HANDLE hObj)
{
    if (hObj.iIndex == 0 || hObj.iIndex >= m_wrapperPool.size())
        return nullptr;

    if (m_dataPool[hObj.iIndex].iVersion != hObj.iVersion)
        return nullptr;

    return m_wrapperPool[hObj.iIndex];
}

bool CGameObject_System::Is_Valid_Handle(GAMEOBJECT_HANDLE hObj) const
{
    if (hObj.iIndex == 0 || hObj.iIndex >= m_dataPool.size())
        return false;

    const auto& tData = m_dataPool[hObj.iIndex];
    return tData.bActive && (tData.iVersion == hObj.iVersion);
}

void CGameObject_System::Free()
{
    for (CGameObject* pWrapper : m_wrapperPool)
        Safe_Release(pWrapper);

    m_wrapperPool.clear();
    m_dataPool.clear();

    while (!m_freeIndices.empty())
        m_freeIndices.pop();

    for (uint32_t i = 0; i < Layer::MAX_LAYERS; ++i)
        m_layerBuckets[i].clear();

    CBase::Free();
}

NS_END
