#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagTestASpec : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::TEST_A);

    _float3 vData[4]{};
}TEST_A_SPEC;


typedef struct ENGINE_DLL tagTransformSpec final : public COMPONENT_SPEC_BASE
{
    _float4x4 World = {};
}TRANSFORM_SPEC;





NS_END
