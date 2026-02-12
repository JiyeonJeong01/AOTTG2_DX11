#pragma once

#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagTransformData final
{
    /* TRS truth */
    _float3     vPosition{ 0,0,0 };
    _float4     vRotationQuat{ 0,0,0,1 };
    _float3     vScale{ 1,1,1 };

    /* Cache */
    _float4x4   matWorld{};

    _bool       bDirty{ true };
}TRANSFORM_DATA;

class CTransform_Processor;

class ENGINE_DLL CTransform final : public CComponent_Proxy_Base<TRANSFORM_DATA, CTransform>
{
public:
    CTransform() : CComponent_Proxy_Base() { m_eComType = COMPONENT_TYPE::TRANSFORM; }
    CTransform(DataType* pData, COMPONENT_HANDLE handle)
    : CComponent_Proxy_Base(pData, handle) { m_eComType = COMPONENT_TYPE::TRANSFORM; }
    ~CTransform() override = default;

public :
    void                Translate(_fvector vWorldDir, SPACE eSpace = SPACE::LOCAL);
    void                Rotate(_fvector vWorldAxis, _float fDegree, SPACE eSpace = SPACE::LOCAL);
    void                Scale(const _float3& vLocalDelta);

    void                Look_At(_fvector vTargetPos);

public:
    _matrix             Get_WorldXM() const;          
    _vector             Get_StateXM(STATE eState) const;

    _float3             Get_Rotation_Euler() const;
    void                Set_Rotation_Euler(_float3 vEulerDegree);
    _float4             Get_Rotation_Quaternion() const;

    _float3             Get_Scale() const;
    void                Set_Scale(const _float3& vScale);

    void                Set_Identity();

    static constexpr float s_EPS = 1e-8f;

};

NS_END
