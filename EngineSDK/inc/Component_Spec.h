#pragma once
#include "Engine_Define.h"
#include "Spec_Struct.h"

NS_BEGIN(Engine)
    typedef struct ENGINE_DLL tagTestASpec : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::TEST_A)

    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override {
        return std::make_unique<tagTestASpec>(*this);
    }

    _float3 vData[4]{};
} TEST_A_SPEC;

typedef struct ENGINE_DLL tagTestBSpec : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::TEST_B)

    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override {
        return std::make_unique<tagTestBSpec>(*this);
    }

    _float3 vData[4]{};
} TEST_B_SPEC;

typedef struct ENGINE_DLL tagTransformSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::TRANSFORM)

    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override {
        return std::make_unique<tagTransformSpec>(*this);
    }

    _float3 vPosition{ 0,0,0 };
    _float4 vRotationQuat{ 0,0,0,1 };
    _float3 vScale{ 1,1,1 };
} TRANSFORM_SPEC;

typedef struct ENGINE_DLL tagTextureSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::TEXTURE)

    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override {
        return std::make_unique<tagTextureSpec>(*this);
    }

    const _tchar* pFilePathPattern = nullptr;
    _uint iNumSRVs = 1;
} TEXTURE_SPEC;

NS_END
