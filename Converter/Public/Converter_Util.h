#pragma once

#include "Converter_Define.h"

NS_BEGIN(Converter)

static inline void CopyFloat3(_float3& dst, const aiVector3D& src)
{
    dst.x = src.x; dst.y = src.y; dst.z = src.z;
}
static inline void CopyFloat2(_float2& dst, const aiVector3D& src)
{
    dst.x = src.x; dst.y = src.y;
}

NS_END
