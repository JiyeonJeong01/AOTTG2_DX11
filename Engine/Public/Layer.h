#pragma once

#include "Base.h"

NS_BEGIN(Engine)

/* TODO: This location may need to be moved later */
using LAYER_ID = uint8_t;
using LAYER_MASK = uint32_t;
constexpr LAYER_ID INVALID_LAYER = 0xff;

class CGameObject;

class CLayer final : public CBase
{
private:
    CLayer();
    virtual ~CLayer() = default;

public:
    HRESULT Add_GameObject(CGameObject* pGameObject);

private:
    vector<CGameObject*>	m_GameObjects;
    LAYER_ID                m_iLayerID{};

public:
    static CLayer* Create();
    void Free() override;
};

NS_END
