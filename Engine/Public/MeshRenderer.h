#pragma once
#include "CComponent_Proxy_Base.h"
#include "Engine_Math.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagMeshRendererData final
{
    OBJECT_HANDLE       hObject{};
    _bool               bEnable = false;

    COMPONENT_HANDLE    hTransform = INVALID_HANDLE;
    COMPONENT_HANDLE    hAnimator = INVALID_HANDLE;
    COMPONENT_HANDLE    hSkinningSourceAnimator = INVALID_HANDLE;
    COMPONENT_HANDLE    hAttachSourceAnimator = INVALID_HANDLE;

    uint32_t            hMesh = INVALID_HANDLE_UINT;
    uint32_t            hMaterial = INVALID_HANDLE_UINT;
    uint32_t            hPerObjectParams = INVALID_HANDLE_UINT;

    uint32_t            flags = RF_NONE;
    RENDER_LAYER        layer = RENDER_LAYER::NONBLEND;

    _float              sortZ = 0.f;

    MESH_MODE               eMode = MESH_MODE::NONE;    /* NONE / PARTS / ATTACH */

    std::vector<uint32_t>   vecSkinningBoneRemap;      /* 파츠 bone index -> 부모 bone index */
    std::vector<_float4x4>  vecSkinningBoneMatrices;   /* 파츠용으로 재조립한 최종 bone matrices */

    uint32_t                iAttachBoneIdx = INVALID_HANDLE_UINT;
    std::string             strAttachBoneName;
    _float4x4               matFinalAttach = Math::Identity();
} MESH_RENDERER_DATA;

class ENGINE_DLL CMeshRenderer : public CComponent_Proxy_Base<MESH_RENDERER_DATA, CMeshRenderer, COMPONENT_TYPE::MESH_RENDERER>
{
public :
    CMeshRenderer() : CComponent_Proxy_Base() { }
    CMeshRenderer(DataType* pData, COMPONENT_HANDLE handle)
        : CComponent_Proxy_Base(pData, handle) {  }
    ~CMeshRenderer() override = default;

public:
    // Settings
    void Set_Mesh(uint32_t h);
    void Set_Material(uint32_t h);

    void Set_Layer(RENDER_LAYER e);
    void Set_Flags(uint32_t f);
    void Add_Flags(uint32_t f);
    void Remove_Flags(uint32_t f);
    void Set_SortZ(float z);

public:
    // Getters
    COMPONENT_HANDLE Get_Transform() const;
    uint32_t         Get_Mesh() const;
    uint32_t         Get_Material() const;
    uint32_t         Get_Flags() const;
    RENDER_LAYER     Get_Layer() const;
    float            Get_SortZ() const;
};

NS_END
