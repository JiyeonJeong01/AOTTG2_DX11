#include "GameObject_System.h"
#include "GameObject.h"
#include "Engine_Log.h"
#include "Transform.h"
#include "RectTransform.h"
#include "LayerHelper.h"
#include "Prototype_Handler.h"
#include "Asset_Registry.h"

#include <atomic>

NS_BEGIN(Engine)

IMPLEMENT_SINGLETON(CGameObject_System)

CGameObject_System::CGameObject_System()
{
}

CGameObject_System::~CGameObject_System()
{
    m_wrapperPool.clear();
    m_dataPool.clear();

    while (!m_freeIndices.empty())
        m_freeIndices.pop();

    for (uint32_t i = 0; i < Layer::MAX_LAYERS; ++i)
        m_layerBuckets[i].clear();
}

HRESULT CGameObject_System::Initialize(uint32_t iMaxLayers, uint32_t iPoolSize)
{
    IF_TRUE_RETURN_MSG_BREAK((iMaxLayers <= Layer::DEFAULT_LAYER || iMaxLayers > Layer::MAX_LAYERS), E_FAIL, "CGameObject_System Initialize failed: invalid layer count.");
    IF_TRUE_RETURN_MSG_BREAK((iPoolSize < 2), E_FAIL, "CGameObject_System Initialize failed: pool size too small.");

    m_iLayerCount = iMaxLayers;

    for (uint32_t i = 0; i < Layer::MAX_LAYERS; ++i)
        m_layerBuckets[i].clear();

    m_dataPool.clear();
    m_wrapperPool.clear();

    m_dataPool.resize(iPoolSize);
    m_wrapperPool.resize(iPoolSize);

    while (!m_freeIndices.empty())
        m_freeIndices.pop();

    /* m_wrapperPool[0] is for DummyData */
    m_wrapperPool[0] = nullptr;

    for (uint32_t i = 1; i < iPoolSize; ++i)
    {
        m_wrapperPool[i] = std::make_unique<CGameObject>(i);

        m_freeIndices.push(i);
        m_dataPool[i].Reset();
    }

    m_pLayerHelper = CLayerHelper::Create(iMaxLayers);

    return S_OK;
}

CGameObject* CGameObject_System::Create_Object_Inner(Layer::LAYER_ID iLayer, const string& strName,
    CGameObject* pParent, const INSTANCE_UUID& tUUID)
{
    IF_TRUE_RETURN_MSG_BREAK((m_freeIndices.empty()), nullptr, "GameObject Pool is Full!");

    if (iLayer == Layer::INVALID_LAYER || iLayer >= m_iLayerCount)
    {
        _DEBUG_WARN("Invalid layer; set to DEFAULT_LAYER.");
        iLayer = Layer::DEFAULT_LAYER;
    }

    /* Allocate index */
    const uint32_t idx = m_freeIndices.front();
    m_freeIndices.pop();

    GAMEOBJECT_DATA& data = m_dataPool[idx];
    CGameObject* pWrapper = m_wrapperPool[idx].get();

    if (!pWrapper)
    {
        _DEBUG_ERROR_BREAK("Create_GameObject failed: wrapper is null.");
        m_freeIndices.push(idx); /* Return allocated index */
        return nullptr;
    }

    /* Reset data */
    data.Reset();
    data.bActive = true;
    data.layer = iLayer;
    data.tUUID = tUUID;

    pWrapper->m_hSelf = OBJECT_HANDLE(idx, data.iVersion, iLayer == Layer::UI_LAYER ? true : false);

    pWrapper->Set_Label(strName);
    if (pParent)
        pWrapper->Set_Parent(pParent);

    Add_To_LayerBucket(pWrapper, iLayer);

    return pWrapper;
}

CGameObject* CGameObject_System::Create_GameObject(Layer::LAYER_ID iLayer, const string& strName, CGameObject* pParent, const INSTANCE_UUID& tUUID)
{
    CGameObject* pWrapper = Create_Object_Inner(iLayer, strName, pParent, tUUID);

    CTransform tr = pWrapper->Add_Component<CTransform>(COMPONENT_TYPE::TRANSFORM);
    if (!tr.Is_Valid())
    {
        /* rollback */
        _DEBUG_ERROR_BREAK("Failed add transform component to GameObject");
        Destroy_Object(pWrapper);
        return nullptr;
    }

    return pWrapper;
}

CGameObject* CGameObject_System::Create_GameObjectUI(Layer::LAYER_ID iLayer, const string& strName, CGameObject* pParent,
    const INSTANCE_UUID& tUUID)
{
    CGameObject* pWrapper = Create_Object_Inner(iLayer, strName, pParent, tUUID);
    CRectTransform tr = pWrapper->Add_Component<CRectTransform>(COMPONENT_TYPE::RECT_TRANSFORM);
    if (!tr.Is_Valid())
    {
        /* rollback */
        _DEBUG_ERROR_BREAK("Failed add rect transform component to UIObject");
        Destroy_Object(pWrapper);
        return nullptr;
    }
    IF_TRUE_RETURN_MSG_BREAK((!pWrapper->Get_Handle().Is_UI()), pWrapper, "Create UIObject, but it has GameObject handle");

    return pWrapper;
}

CGameObject* CGameObject_System::Instantiate(const string& strProto, Layer::LAYER_ID iLayer, const string& strName, CGameObject* pParent)
{
    const CPrototype* pProto = SYS_ASSET.Prototypes().Find(strProto);
    IF_NULL_RETURN_MSG_BREAK(pProto, nullptr, "Can't find such prototype");

    CGameObject* pObj = pProto->Clone(iLayer, strName, INSTANCE_UUID::New());
    IF_NULL_RETURN_MSG_BREAK(pObj, nullptr, "pObj is nullptr");

    pObj->Set_Parent(pParent);
    return     pObj;
}

CGameObject* CGameObject_System::Instantiate(const ASSET_GUID& tGUID, Layer::LAYER_ID iLayer, const string& strName, CGameObject* pParent)
{
    const CPrototype* pProto = SYS_ASSET.Prototypes().Find(tGUID);
    IF_NULL_RETURN_MSG_BREAK(pProto, nullptr, "Can't find such prototype");

    CGameObject* pObj = pProto->Clone(iLayer, strName, INSTANCE_UUID::New());
    IF_NULL_RETURN_MSG_BREAK(pObj, nullptr, "pObj is nullptr");

    pObj->Set_Parent(pParent);
    return     pObj;
}

void CGameObject_System::Destroy_Object(CGameObject* pObj)
{
    IF_NULL_RETURN_MSG_BREAK(pObj, , "Destroy_Object failed : pObj is nullptr");
    IF_TRUE_RETURN_MSG_BREAK(!pObj->Is_Valid(), , "Destroy_Object failed : pObj is invalid");

    const OBJECT_HANDLE handle = pObj->Get_Handle();
    if (handle.Index() == 0 /* dummy */ || handle.Index() >= m_dataPool.size())
        return;

    GAMEOBJECT_DATA& data = m_dataPool[handle.Index()];

    if (data.bPendingDestroy)
        return;
    data.bPendingDestroy = true;
    m_pendingDestroys.push_back(handle.Index());

    Remove_From_LayerBucket(pObj);

    /* Clean up the relationship with the parent. */
    if (data.hParent.Is_Valid())
    {
        CGameObject* pParent = Get_Wrapper(data.hParent);
        if (pParent && pParent->Is_Valid())
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
            if (pChild && pChild->Is_Valid())
            {
                pChild->Set_Parent(nullptr);
            }
        }
    }

    /* NOTE : Pending 시 주의할 로직 */
    data.bActive = false;
}

void CGameObject_System::Set_Layer(CGameObject* pObj, Layer::LAYER_ID iNewLayer)
{
    IF_NULL_RETURN_MSG_BREAK(pObj, , "Destroy_Object failed : pObj is nullptr");
    IF_TRUE_RETURN_MSG_BREAK(!pObj->Is_Valid(), , "Destroy_Object failed : pObj is invalid");

    if (iNewLayer >= m_iLayerCount || iNewLayer == Layer::INVALID_LAYER)
    {
        _DEBUG_ERROR_BREAK("Invalid layer index; set to DEFAULT_LAYER.");
        iNewLayer = Layer::DEFAULT_LAYER;
    }

    OBJECT_HANDLE hObj = pObj->Get_Handle();
    GAMEOBJECT_DATA& data = Access_Data_Raw(hObj);
    if (data.layer == iNewLayer)
        return;

    Remove_From_LayerBucket(pObj);
    Add_To_LayerBucket(pObj, iNewLayer);

    if (iNewLayer == Layer::UI_LAYER)  pObj->m_hSelf.raw |= OBJECT_HANDLE::UI_MASK;
    else                               pObj->m_hSelf.raw &= ~OBJECT_HANDLE::UI_MASK;
}

const std::vector<CGameObject*>& CGameObject_System::Get_LayerObjects(Layer::LAYER_ID iLayer) const
{
    static const std::vector<CGameObject*> s_Empty;
    IF_TRUE_RETURN_MSG_BREAK((iLayer >= m_iLayerCount || iLayer == Layer::INVALID_LAYER), s_Empty, "Invalid layer index; returning empty.");

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
            if (pObj && pObj->Is_Valid())
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
            if (!pObj || !pObj->Is_Valid())
                continue;

            if (!Access_Data_Raw(pObj->Get_Handle()).hParent.Is_Valid())
                outRoots.push_back(pObj);
        }
    }
}

void CGameObject_System::Set_UUID(CGameObject* pObj, const INSTANCE_UUID& tUUID)
{
    if (!pObj || !pObj->Is_Valid())
        return;

    GAMEOBJECT_DATA& tData = Access_Data_Raw(pObj->Get_Handle());

    tData.tUUID = tUUID;
}

const INSTANCE_UUID& CGameObject_System::Get_UUID(CGameObject* pObj)
{
    if (!pObj || !pObj->Is_Valid())
        return DUMMY_UUID;

    GAMEOBJECT_DATA& tData = Access_Data_Raw(pObj->Get_Handle());
    return tData.tUUID;
}

void CGameObject_System::Remove_From_LayerBucket(CGameObject* pObj)
{
    if (!pObj || !pObj->Is_Valid())
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

        if (pMoved && pMoved->Is_Valid())
            Access_Data_Raw(pMoved->Get_Handle()).iIndexInLayer = iRemoveIdx;
    }

    bucket.pop_back();

    /* Cleanup for the object being removed. */
    tData.layer = Layer::INVALID_LAYER;
    tData.iIndexInLayer = 0;
}


void CGameObject_System::Add_To_LayerBucket(CGameObject* pObj, Layer::LAYER_ID layer)
{
    if (!pObj || !pObj->Is_Valid())
        return;

    if (layer == Layer::INVALID_LAYER || layer >= m_iLayerCount)
        layer = Layer::DEFAULT_LAYER;

    const OBJECT_HANDLE hObj = pObj->Get_Handle();
    GAMEOBJECT_DATA& data = Access_Data_Raw(hObj);

    /* Remove this object from its current layer bucket. */
    if (data.layer != Layer::INVALID_LAYER && data.layer != layer)
        Remove_From_LayerBucket(pObj);

    auto& bucket = m_layerBuckets[layer];

    data.layer = layer;
    data.iIndexInLayer = (uint32_t)bucket.size();

    bucket.push_back(pObj);
}

void CGameObject_System::Flush_PendingDestroy()
{
    if (m_pendingDestroys.empty())
        return;

    for (uint32_t idx : m_pendingDestroys)
    {
        if (idx == 0 || idx >= static_cast<uint32_t>(m_dataPool.size()))
        {
            _DEBUG_ERROR_BREAK("Flush_PendingDestroy: invalid index.");
            continue;
        }

        GAMEOBJECT_DATA& data = m_dataPool[idx];

        if (!data.bPendingDestroy)
            continue;

        CGameObject* pObj = m_wrapperPool[idx].get();
        if (pObj)
            pObj->Remove_All_Components();
        else
            _DEBUG_ERROR_BREAK("Flush_PendingDestroy: wrapper is null.");

        data.iVersion++;
        data.Reset();
        m_freeIndices.push(idx);
    }

    m_pendingDestroys.clear();
}

GAMEOBJECT_DATA& CGameObject_System::Access_Data_Raw(OBJECT_HANDLE hObj)
{
    IF_TRUE_RETURN_MSG_BREAK((hObj.Index() == 0 || hObj.Index() >= m_dataPool.size()), m_dataPool[0], "Access_Data_Raw failed: invalid index.");
    return m_dataPool[hObj.Index()];
}

CGameObject* CGameObject_System::Get_Wrapper(OBJECT_HANDLE hObj)
{
    if (hObj.Index() == 0 || hObj.Index() >= m_wrapperPool.size())
        return nullptr;

    const auto& data = m_dataPool[hObj.Index()];
    if (!data.bActive)
        return nullptr;

    if (data.iVersion != hObj.Version())
        return nullptr;

    return m_wrapperPool[hObj.Index()].get();
}

bool CGameObject_System::Is_Valid_Handle(OBJECT_HANDLE hObj) const
{

    if (hObj.Index() == 0 || hObj.Index() >= m_dataPool.size())
        return false;

    const auto& tData = m_dataPool[hObj.Index()];
    return tData.bActive && (tData.iVersion == hObj.Version());
}

HRESULT CGameObject_System::Build_SceneSpecs(std::vector<SCENE_OBJECT_SPEC>& outSpecs)
{
    outSpecs.clear();
    outSpecs.reserve(m_dataPool.size() - m_freeIndices.size() - 1);

    size_t iTotalObjects = m_dataPool.size();
    for (size_t i = 0; i < iTotalObjects; ++i)
    {
        auto pObj = m_wrapperPool[i].get();
        if (!pObj || !pObj->Is_Valid())
            continue;
        auto tData = m_dataPool[i];

        SCENE_OBJECT_SPEC spec;
        spec.uuid = Get_UUID(pObj);
        spec.name = pObj->Get_Label();
        spec.isUI = pObj->Get_Handle().Is_UI();
        spec.parent = pObj->Get_Parent() == nullptr ? INSTANCE_UUID{} : Get_UUID(pObj->Get_Parent());
        spec.layer = pObj->Get_Layer();
        spec.protoGuid = tData.tProtoGUID;

        Component::COMPONENT_MASK mask = pObj->Get_ComponentMask();
        for (_uint j = 0; j < COMPONENT_MAX; ++j)
        {
            if ((mask & Component::Component_Bit(INT_TO_COM(j))) == 0)
                continue;

            vector<COMPONENT_HANDLE> hComponents;
            SYS_COMPONENT.Get_Component_Handle_By_Type(INT_TO_COM(j), pObj->Get_Handle(), hComponents);
            for (auto hCom : hComponents)
                spec.overrides.components[j] =  SYS_COMPONENT.Build_Spec_By_Type(INT_TO_COM(j), hCom);
            __noop;
        }

        outSpecs.emplace_back(std::move(spec));

    }
    return S_OK;
}


CLayerHelper& CGameObject_System::Layers() const
{
    return *(m_pLayerHelper.get());
}

NS_END
