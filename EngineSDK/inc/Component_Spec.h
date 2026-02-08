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
    _float3     vPosition{ 0,0,0 };
    _float4     vRotationQuat{ 0,0,0,1 };
    _float3     vScale{ 1,1,1 };
}TRANSFORM_SPEC;


typedef struct ENGINE_DLL tagTextureSpec final : public COMPONENT_SPEC_BASE
{
    const _tchar* pFilePathPattern = nullptr;
    _uint         iNumSRVs = 1;

}TEXTURE_SPEC;


NS_END
