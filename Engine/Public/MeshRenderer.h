#pragma once
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagMeshRendererData final
{
    OBJECT_HANDLE       hObject{ };
    _bool               bEnable = false;

    COMPONENT_HANDLE hTransform = INVALID_HANDLE;
    COMPONENT_HANDLE hAnimator = INVALID_HANDLE;
    uint32_t hMesh =            INVALID_HANDLE_UINT;
    uint32_t hMaterial =        INVALID_HANDLE_UINT;
    uint32_t hPerObjectParams = INVALID_HANDLE_UINT;

    uint32_t     flags = RF_NONE;
    RENDER_LAYER layer = RENDER_LAYER::NONBLEND;

    _float    sortZ = 0.f; // transparent/ui
}MESH_RENDERER_DATA;

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
