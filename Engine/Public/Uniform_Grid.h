#pragma once

#include "Engine_Define.h"
#include "Physics_Struct.h"

NS_BEGIN(Engine)

class CUniform_Grid final
{
public:
    CUniform_Grid();
    ~CUniform_Grid();

public:
    HRESULT Initialize(const _float3& vWorldMin, const _float3& vWorldMax, _float fCellSize);

    void    Clear();
    void    Insert_Static(COMPONENT_HANDLE hCollider, const AABB& tAABB);
    void    Query_StaticOverlap(const AABB& tAABB, _Out_ std::vector<COMPONENT_HANDLE>* pOutOverlaps);

public:
    void    Clear_Debug_State();

    void    Set_DebugDraw_AllCells(_bool bEnable) { m_bDebugDrawAllCells = bEnable; }
    void    Set_DebugDraw_OccupiedCells(_bool bEnable) { m_bDebugDrawOccupiedCells = bEnable; }
    void    Set_DebugDraw_QueriedCells(_bool bEnable) { m_bDebugDrawQueriedCells = bEnable; }
    void    Set_DebugDraw_QueryAABB(_bool bEnable) { m_bDebugDrawQueryAABB = bEnable; }

    _bool   Get_DebugDraw_AllCells() const { return m_bDebugDrawAllCells; }
    _bool   Get_DebugDraw_OccupiedCells() const { return m_bDebugDrawOccupiedCells; }
    _bool   Get_DebugDraw_QueriedCells() const { return m_bDebugDrawQueriedCells; }
    _bool   Get_DebugDraw_QueryAABB() const { return m_bDebugDrawQueryAABB; }

    void    Begin_Debug_Frame();
    const   std::vector<UNIFORM_GRID_DEBUG_CELL>& Get_DebugFrameQueriedCells() const { return m_vecDebugFrameQueriedCells; }

public:
    _float3 Get_WorldMin() const { return m_vWorldMin; }
    _float3 Get_WorldMax() const { return m_vWorldMax; }
    _float  Get_CellSize() const { return m_fCellSize; }

    int     Get_DimX() const { return m_iGridDimX; }
    int     Get_DimY() const { return m_iGridDimY; }
    int     Get_DimZ() const { return m_iGridDimZ; }

    _bool   Has_Any_StaticCollider(int iX, int iY, int iZ) const;
    _uint   Get_StaticColliderCount(int iX, int iY, int iZ) const;

    void    Calc_CellMinMax(int iX, int iY, int iZ, _Out_ _float3* pOutMin, _Out_ _float3* pOutMax) const;

private:
    void    Calc_RangeInCell(const AABB& tAABB, GRID_COORD* pOutMin, GRID_COORD* pOutMax) const;
    size_t  ToIndex(int iX, int iY, int iZ) const;

public:
    void    Clear_Dynamic();
    void    Insert_Dynamic(COMPONENT_HANDLE hCollider, const AABB& tAABB);
    void    Query_DynamicOverlap(const AABB& tAABB, _Out_ std::vector<COMPONENT_HANDLE>* pOutOverlaps);

    _bool   Has_Any_DynamicCollider(int iX, int iY, int iZ) const;
    _uint   Get_DynamicColliderCount(int iX, int iY, int iZ) const;

private:
    _float3 m_vWorldMin{};
    _float3 m_vWorldMax{};
    _float  m_fCellSize = 1.f;

    int     m_iGridDimX = 1;
    int     m_iGridDimY = 1;
    int     m_iGridDimZ = 1;

    std::vector<std::vector<COMPONENT_HANDLE>> m_StaticColliders;
    std::vector<std::vector<COMPONENT_HANDLE>> m_DynamicColliders;
private:
    _bool   m_bDebugDrawAllCells = false;
    _bool   m_bDebugDrawOccupiedCells = true;
    _bool   m_bDebugDrawQueriedCells = true;
    _bool   m_bDebugDrawQueryAABB = true;

    std::vector<UNIFORM_GRID_DEBUG_CELL>    m_vecDebugFrameQueriedCells;
    std::unordered_set<size_t>              m_setDebugFrameQueriedCellIndices;

public :
    static std::unique_ptr<CUniform_Grid> Create(const _float3& vWorldMin, const _float3& vWorldMax, _float fCellSize);
};

NS_END
