#pragma once

#include "Engine_Define.h"

NS_BEGIN(Engine)

class ENGINE_DLL CLayerHelper final
{
public :
    CLayerHelper(_uint iLayerCount);
    ~CLayerHelper();
public :
    HRESULT Initialize();

    inline _bool    Is_Used(Layer::LAYER_ID layer);
    _bool           Register_Layer(Layer::LAYER_ID layer, std::string strName);
    void            Gather_UnusedLayers(vector<Layer::LAYER_ID>& unusedLayers);
    _bool           Find_Layer(const std::string& strName, Layer::LAYER_ID& outLayer);
    _bool           Find_Name(Layer::LAYER_ID layer, std::string& outStrName);

    _bool           Rename_Layer(Layer::LAYER_ID layer, const std::string& newName);
    _bool           Unregister_Layer(Layer::LAYER_ID layer);

private:
    Layer::LAYER_MASK                       m_UsedLayerMask{};
    _uint                                   m_iTotalLayerCnt{};

    std::map<Layer::LAYER_ID, std::string> m_LayerToName;

public :
    static std::unique_ptr<CLayerHelper> Create(_uint iLayerCount = Layer::MAX_LAYERS);
};



NS_END
