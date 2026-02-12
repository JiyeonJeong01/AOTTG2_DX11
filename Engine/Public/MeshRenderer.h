#pragma once
#include "CComponent_Proxy_Base.h"

typedef struct ENGINE_DLL tagMeshRendererData final
{
    COMPONENT_HANDLE hTransform{};
    uint32_t hMesh = 0;
    uint32_t hMaterial = 0;

    uint32_t     flags = RF_NONE;
    RENDER_LAYER layer = RENDER_LAYER::NONBLEND;

    _float    sortZ = 0.f; // transparent/ui
    uint8_t  bEnabled = 1;
    uint8_t  pad[3] = {};
}MESH_RENDERER_DATA;

class CMeshRenderer : public CComponent_Proxy_Base<MESH_RENDERER_DATA, CMeshRenderer>
{
public :
    using DataType = MESH_RENDERER_DATA;

    CMeshRenderer() : CComponent_Proxy_Base() { m_eComType = COMPONENT_TYPE::MESH_RENDERER; }
    CMeshRenderer(DataType* pData, COMPONENT_HANDLE handle)
        : CComponent_Proxy_Base(pData, handle) {  m_eComType = COMPONENT_TYPE::MESH_RENDERER; }
    ~CMeshRenderer() override = default;

public :
    void Set_Enabled(_bool b) { m_pData->bEnabled = b ? 1 : 0; }
    bool Is_Enabled() const { return m_pData->bEnabled != 0; }

    void Set_Mesh(uint32_t h) { m_pData->hMesh = h; }
    void Set_Material(uint32_t h) { m_pData->hMaterial = h; }

    void Set_Layer(RENDER_LAYER e) { m_pData->layer = e; }
    void Set_Flags(uint32_t f) { m_pData->flags = f; }
    void Add_Flags(uint32_t f) { m_pData->flags |= f; }
    void Remove_Flags(uint32_t f) { m_pData->flags &= ~f; }

    void Set_SortZ(float z) { m_pData->sortZ = z; }

    COMPONENT_HANDLE    Get_Transform() const { return m_pData->hTransform; }
    uint32_t            Get_Mesh() const { return m_pData->hMesh; }
    uint32_t            Get_Material() const { return m_pData->hMaterial; }
    uint32_t            Get_Flags() const { return m_pData->flags; }
    RENDER_LAYER        Get_Layer() const { return m_pData->layer; }

};

