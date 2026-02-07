#include "Transform.h"
#include "Engine_MathDX.h"

const _float4x4& CTransform::Get_World() const
{
    return m_pData->World;
}

void CTransform::Set_World(const _float4x4& world)
{
    m_pData->World = world;
}

_matrix CTransform::Get_WorldXM() const
{
    return MathDX::Load(m_pData->World);
}

void CTransform::Set_WorldXM(_fmatrix world)
{
    MathDX::Store(m_pData->World, world);
}

TYPE_TIP t;
_vector CTransform::Get_StateXM(STATE axis) const
{
    const _float4* fState = reinterpret_cast<const _float4*>(&m_pData->World.m[SCAST(_uint, axis)][0]);
    return MathDX::Load(*fState);
}

void CTransform::Set_StateXM(STATE axis, _fvector vAxis)
{
}

_float3 CTransform::Get_Scaled() const
{
}

void CTransform::Set_Identity()
{
}
