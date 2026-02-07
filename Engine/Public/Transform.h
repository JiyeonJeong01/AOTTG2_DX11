#pragma once

#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

struct TRANSFORM_DATA final
{
    _float4x4    World{};            // storage/serialization
    _bool        bDirty{ true };
};

class CTransform_Processor;

class ENGINE_DLL CTransform final : public CComponent_Proxy_Base<TRANSFORM_DATA, CTransform>
{
public:
    using ProcessorType = CTransform_Processor;
    using DataType = TRANSFORM_DATA;

public:
    CTransform() : CComponent_Proxy_Base(COMPONENT_TYPE::TRANSFORM) { }
    CTransform(COMPONENT_TYPE eType, DataType* pData, COMPONENT_HANDLE handle)
    : CComponent_Proxy_Base(eType, pData, handle) { }
    ~CTransform() override = default;

public:
    const _float4x4& Get_World() const;
    void             Set_World(const _float4x4& world);

    _matrix          Get_WorldXM() const;          
    void             Set_WorldXM(_fmatrix world);  

public:
    _vector          Get_StateXM(STATE axis) const;
    void             Set_StateXM(STATE axis, _fvector vAxis);

    _float3          Get_Scaled() const;

public:
    void             Set_Identity();

};

NS_END
