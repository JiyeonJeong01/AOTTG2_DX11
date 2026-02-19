#include "LayerHelper.h"
#include "Engine_Log.h"

CLayerHelper::CLayerHelper(_uint iLayerCount)
    : m_iTotalLayerCnt(iLayerCount)
{
}

CLayerHelper::~CLayerHelper()
{
}

HRESULT CLayerHelper::Initialize()
{
    Layer::UI_LAYER = m_iTotalLayerCnt - 1;
    Register_Layer(Layer::DEFAULT_LAYER, "Default");
    Register_Layer(Layer::UI_LAYER, "UI");

    return S_OK;
}

_bool CLayerHelper::Is_Used(Layer::LAYER_ID layer)
{
    return ((m_UsedLayerMask & Layer::To_Bit(layer)) != 0);
}

_bool CLayerHelper::Register_Layer(Layer::LAYER_ID layer, std::string strName)
{
    if (Is_Used(layer))
        return false;

    m_UsedLayerMask |= Layer::To_Bit(layer);
    return m_LayerToName.emplace(layer, std::move(strName)).second; /* If success, return true */
}

void CLayerHelper::Gather_UnusedLayers(vector<Layer::LAYER_ID>& unusedLayers)
{
    unusedLayers.clear();
    unusedLayers.reserve(m_iTotalLayerCnt);
    for (Layer::LAYER_ID i = 0; i < m_iTotalLayerCnt; ++i)
    {
        if (!Is_Used(i))
            unusedLayers.push_back(i);
    }
}

_bool CLayerHelper::Find_Layer(const std::string& strName, Layer::LAYER_ID& outLayer)
{
    auto it = std::find_if(m_LayerToName.begin(), m_LayerToName.end(),
        [&](const auto& pair) -> _bool {
            return (pair.second == strName);
        });

    if (it == m_LayerToName.end())
        return false;

    outLayer = it->first;
    return true;
}

_bool CLayerHelper::Find_Name(Layer::LAYER_ID layer, std::string& outStrName)
{
    if (!Is_Used(layer))
        return false;

    outStrName = m_LayerToName[layer];
    return true;
}

_bool CLayerHelper::Rename_Layer(Layer::LAYER_ID layer, const std::string& newName)
{
    if (layer == Layer::DEFAULT_LAYER || layer == Layer::UI_LAYER || !Is_Used(layer))
        return false;

    for (const auto& [layer, name] : m_LayerToName)
        if (name == newName)
            return false;

    auto it = m_LayerToName.find(layer);
    if (it == m_LayerToName.end())
        return false;

    it->second = newName;
    return true;
}

_bool CLayerHelper::Unregister_Layer(Layer::LAYER_ID layer)
{
    if (layer == Layer::DEFAULT_LAYER || layer == Layer::UI_LAYER || !Is_Used(layer))
        return false;

    auto it = m_LayerToName.find(layer);
    if (it == m_LayerToName.end())
        return false;

    m_LayerToName.erase(it);
    m_UsedLayerMask &= ~Layer::To_Bit(layer);

    return true;
}

std::unique_ptr<CLayerHelper> CLayerHelper::Create(_uint iLayerCount)
{
    auto pInstance = std::make_unique<CLayerHelper>(iLayerCount);
    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Instance Create failed");
    return pInstance;
}
