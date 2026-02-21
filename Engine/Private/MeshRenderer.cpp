#include "MeshRenderer.h"

NS_BEGIN(Engine)

void CMeshRenderer::Set_Enabled(_bool b)
{
    m_pData->bEnabled = b ? 1 : 0;
}

bool CMeshRenderer::Is_Enabled() const
{
    return m_pData->bEnabled != 0;
}

void CMeshRenderer::Set_Mesh(uint32_t h)
{
    m_pData->hMesh = h;
}

void CMeshRenderer::Set_Material(uint32_t h)
{
    m_pData->hMaterial = h;
}


void CMeshRenderer::Set_MainTexture(uint32_t hTexture)
{
    m_pData->hMainTex = hTexture;
}

uint32_t CMeshRenderer::Get_MainTexture() const
{
    return m_pData->hMainTex;
}
void CMeshRenderer::Set_Layer(RENDER_LAYER e)
{
    m_pData->layer = e;
}

void CMeshRenderer::Set_Flags(uint32_t f)
{
    m_pData->flags = f;
}

void CMeshRenderer::Add_Flags(uint32_t f)
{
    m_pData->flags |= f;
}

void CMeshRenderer::Remove_Flags(uint32_t f)
{
    m_pData->flags &= ~f;
}

void CMeshRenderer::Set_SortZ(float z)
{
    m_pData->sortZ = z;
}

COMPONENT_HANDLE CMeshRenderer::Get_Transform() const
{
    return m_pData->hTransform;
}

uint32_t CMeshRenderer::Get_Mesh() const
{
    return m_pData->hMesh;
}

uint32_t CMeshRenderer::Get_Material() const
{
    return m_pData->hMaterial;
}

uint32_t CMeshRenderer::Get_Flags() const
{
    return m_pData->flags;
}

RENDER_LAYER CMeshRenderer::Get_Layer() const
{
    return m_pData->layer;
}

float CMeshRenderer::Get_SortZ() const
{
    return m_pData->sortZ;
}

NS_END
