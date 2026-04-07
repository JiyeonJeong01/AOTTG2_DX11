#include "MeshRenderer.h"
#include "Component_System.h"

NS_BEGIN(Engine)

void CMeshRenderer::Set_Mesh(uint32_t h)
{
    m_pData->hMesh = h;
}

void CMeshRenderer::Set_Material(uint32_t h)
{
    m_pData->hMaterial = h;
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

void CMeshRenderer::Set_Mode(MESH_MODE e)
{
    if (!_Data())
        return;

    _Data()->eMode = e;
}

void CMeshRenderer::Set_Particle(uint32_t hParticle)
{
    if (!_Data())
        return;

    _Data()->hParticle = hParticle;
}

void CMeshRenderer::Set_ParticlePlaying(_bool bPlaying)
{
    if (!_Data())
        return;

    _Data()->bParticlePlaying = bPlaying;
}

void CMeshRenderer::Set_ParticlePivot(const _float3& vPivot)
{
    if (!_Data())
        return;

    _Data()->vParticlePivot = vPivot;
}

MESH_MODE CMeshRenderer::Get_Mode() const
{
    if (!_Data())
        return MESH_MODE::NONE;

    return _Data()->eMode;
}

uint32_t CMeshRenderer::Get_Particle() const
{
    if (!_Data())
        return INVALID_HANDLE_UINT;

    return _Data()->hParticle;
}

_bool CMeshRenderer::Get_ParticlePlaying() const
{
    if (!_Data())
        return false;

    return _Data()->bParticlePlaying;
}

const _float3& CMeshRenderer::Get_ParticlePivot() const
{
    static const _float3 vDefault{};
    if (!_Data())
        return vDefault;

    return _Data()->vParticlePivot;
}
NS_END
