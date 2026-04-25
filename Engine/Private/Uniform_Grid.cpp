#include "Uniform_Grid.h"

NS_BEGIN(Engine)
    CUniform_Grid::CUniform_Grid()
    {
    }

    CUniform_Grid::~CUniform_Grid()
    {
    }

HRESULT CUniform_Grid::Initialize(const _float3& vWorldMin, const _float3& vWorldMax, _float fCellSize)
{
    if (fCellSize <= 0.f)
        return E_FAIL;

    m_vWorldMin = vWorldMin;
    m_vWorldMax = vWorldMax;
    m_fCellSize = fCellSize;

    m_iGridDimX = max(1, To<_int>(floorf((vWorldMax.x - vWorldMin.x) / fCellSize)) + 1);
    m_iGridDimY = max(1, To<_int>(floorf((vWorldMax.y - vWorldMin.y) / fCellSize)) + 1);
    m_iGridDimZ = max(1, To<_int>(floorf((vWorldMax.z - vWorldMin.z) / fCellSize)) + 1);

    m_StaticColliders.clear();
    m_StaticColliders.resize(To<size_t>(m_iGridDimX) * m_iGridDimY * m_iGridDimZ);

    m_DynamicColliders.clear();
    m_DynamicColliders.resize(static_cast<size_t>(m_iGridDimX) * m_iGridDimY * m_iGridDimZ);

    Set_DebugDraw_AllCells(false);
    Set_DebugDraw_OccupiedCells(false);
    Set_DebugDraw_QueriedCells(false);
    Set_DebugDraw_QueryAABB(false);

    Clear_Debug_State();

    return S_OK;
}

void CUniform_Grid::Clear()
{
    for (auto& cell : m_StaticColliders)
        cell.clear();

    Clear_Debug_State();
}

void CUniform_Grid::Insert_Static(COMPONENT_HANDLE hCollider, const AABB& tAABB)
{
    if (hCollider == INVALID_HANDLE)
        return;

    GRID_COORD coordMin{};
    GRID_COORD coordMax{};
    Calc_RangeInCell(tAABB, &coordMin, &coordMax);

    for (int iZ = coordMin.iZ; iZ <= coordMax.iZ; ++iZ)
        for (int iY = coordMin.iY; iY <= coordMax.iY; ++iY)
            for (int iX = coordMin.iX; iX <= coordMax.iX; ++iX)
                m_StaticColliders[ToIndex(iX, iY, iZ)].push_back(hCollider);
}

void CUniform_Grid::Query_StaticOverlap(const AABB& tAABB, _Out_ std::vector<COMPONENT_HANDLE>* pOutOverlaps)
{
    if (!pOutOverlaps)
        return;

    pOutOverlaps->clear();

    std::unordered_set<uint32_t> setAddedHandles;

    GRID_COORD coordMin{};
    GRID_COORD coordMax{};
    Calc_RangeInCell(tAABB, &coordMin, &coordMax);

    for (int iZ = coordMin.iZ; iZ <= coordMax.iZ; ++iZ)
    {
        for (int iY = coordMin.iY; iY <= coordMax.iY; ++iY)
        {
            for (int iX = coordMin.iX; iX <= coordMax.iX; ++iX)
            {
                const size_t iCellIndex = ToIndex(iX, iY, iZ);
                const auto& vecCols = m_StaticColliders[iCellIndex];

                if (m_setDebugFrameQueriedCellIndices.insert(iCellIndex).second)
                {
                    UNIFORM_GRID_DEBUG_CELL tCell{};
                    tCell.iX = iX;
                    tCell.iY = iY;
                    tCell.iZ = iZ;
                    tCell.iCellIndex = iCellIndex;
                    tCell.iNumColliders = static_cast<_uint>(vecCols.size());
                    m_vecDebugFrameQueriedCells.push_back(tCell);
                }

                for (COMPONENT_HANDLE hCollider : vecCols)
                {
                    if (!setAddedHandles.insert(hCollider.iHandle).second)
                        continue;

                    pOutOverlaps->push_back(hCollider);
                }
            }
        }
    }
}

void CUniform_Grid::Clear_Debug_State()
{
    m_vecDebugFrameQueriedCells.clear();
    m_setDebugFrameQueriedCellIndices.clear();
}

void CUniform_Grid::Begin_Debug_Frame()
{
    m_vecDebugFrameQueriedCells.clear();
    m_setDebugFrameQueriedCellIndices.clear();
}

_bool CUniform_Grid::Has_Any_StaticCollider(int iX, int iY, int iZ) const
{
    if (iX < 0 || iX >= m_iGridDimX ||
        iY < 0 || iY >= m_iGridDimY ||
        iZ < 0 || iZ >= m_iGridDimZ)
        return false;

    return !m_StaticColliders[ToIndex(iX, iY, iZ)].empty();
}

_uint CUniform_Grid::Get_StaticColliderCount(int iX, int iY, int iZ) const
{
    if (iX < 0 || iX >= m_iGridDimX ||
        iY < 0 || iY >= m_iGridDimY ||
        iZ < 0 || iZ >= m_iGridDimZ)
        return 0;

    return static_cast<_uint>(m_StaticColliders[ToIndex(iX, iY, iZ)].size());
}

void CUniform_Grid::Calc_CellMinMax(int iX, int iY, int iZ, _Out_ _float3* pOutMin, _Out_ _float3* pOutMax) const
{
    if (!pOutMin || !pOutMax)
        return;

    pOutMin->x = m_vWorldMin.x + iX * m_fCellSize;
    pOutMin->y = m_vWorldMin.y + iY * m_fCellSize;
    pOutMin->z = m_vWorldMin.z + iZ * m_fCellSize;

    pOutMax->x = pOutMin->x + m_fCellSize;
    pOutMax->y = pOutMin->y + m_fCellSize;
    pOutMax->z = pOutMin->z + m_fCellSize;
}

void CUniform_Grid::Calc_RangeInCell(const AABB& tAABB, GRID_COORD* pOutMin, GRID_COORD* pOutMax) const
{
    if (!pOutMin || !pOutMax)
        return;

    auto ToCell = [&](float fPos, float fMin, int iDim) -> int
        {
            int iGrid = static_cast<int>(floorf((fPos - fMin) / m_fCellSize));

            if (iGrid < 0)
                iGrid = 0;
            if (iGrid >= iDim)
                iGrid = iDim - 1;

            return iGrid;
        };

    pOutMin->iX = ToCell(tAABB.vMin.x, m_vWorldMin.x, m_iGridDimX);
    pOutMin->iY = ToCell(tAABB.vMin.y, m_vWorldMin.y, m_iGridDimY);
    pOutMin->iZ = ToCell(tAABB.vMin.z, m_vWorldMin.z, m_iGridDimZ);

    pOutMax->iX = ToCell(tAABB.vMax.x, m_vWorldMin.x, m_iGridDimX);
    pOutMax->iY = ToCell(tAABB.vMax.y, m_vWorldMin.y, m_iGridDimY);
    pOutMax->iZ = ToCell(tAABB.vMax.z, m_vWorldMin.z, m_iGridDimZ);
}

size_t CUniform_Grid::ToIndex(int iX, int iY, int iZ) const
{
    return static_cast<size_t>(
        iX +
        iY * m_iGridDimX +
        iZ * m_iGridDimX * m_iGridDimY);
}

void CUniform_Grid::Clear_Dynamic()
{
    for (auto& vecCell : m_DynamicColliders)
        vecCell.clear();
}

void CUniform_Grid::Insert_Dynamic(COMPONENT_HANDLE hCollider, const AABB& tAABB)
{
    if (hCollider == INVALID_HANDLE)
        return;

    GRID_COORD tMin{};
    GRID_COORD tMax{};
    Calc_RangeInCell(tAABB, &tMin, &tMax);

    for (int iZ = tMin.iZ; iZ <= tMax.iZ; ++iZ)
    {
        for (int iY = tMin.iY; iY <= tMax.iY; ++iY)
        {
            for (int iX = tMin.iX; iX <= tMax.iX; ++iX)
            {
                m_DynamicColliders[ToIndex(iX, iY, iZ)].push_back(hCollider);
            }
        }
    }
}

void CUniform_Grid::Query_DynamicOverlap(const AABB& tAABB, _Out_ std::vector<COMPONENT_HANDLE>* pOutOverlaps)
{
    if (!pOutOverlaps)
        return;

    pOutOverlaps->clear();

    std::unordered_set<uint32_t> setAddedHandles;

    GRID_COORD tMin{};
    GRID_COORD tMax{};
    Calc_RangeInCell(tAABB, &tMin, &tMax);

    for (int iZ = tMin.iZ; iZ <= tMax.iZ; ++iZ)
    {
        for (int iY = tMin.iY; iY <= tMax.iY; ++iY)
        {
            for (int iX = tMin.iX; iX <= tMax.iX; ++iX)
            {
                const size_t iIndex = ToIndex(iX, iY, iZ);
                const auto& vecCell = m_DynamicColliders[iIndex];

                if (m_bDebugDrawQueriedCells)
                {
                    if (m_setDebugFrameQueriedCellIndices.insert(iIndex).second)
                    {
                        UNIFORM_GRID_DEBUG_CELL tCell{};
                        tCell.iX = iX;
                        tCell.iY = iY;
                        tCell.iZ = iZ;
                        tCell.iCellIndex = iIndex;
                        tCell.iNumColliders = static_cast<_uint>(vecCell.size());
                        m_vecDebugFrameQueriedCells.push_back(tCell);
                    }
                }

                for (COMPONENT_HANDLE hCollider : vecCell)
                {
                    if (!setAddedHandles.insert(hCollider.iHandle).second)
                        continue;

                    pOutOverlaps->push_back(hCollider);
                }
            }
        }
    }
}


_bool CUniform_Grid::Has_Any_DynamicCollider(int iX, int iY, int iZ) const
{
    if (iX < 0 || iX >= m_iGridDimX ||
        iY < 0 || iY >= m_iGridDimY ||
        iZ < 0 || iZ >= m_iGridDimZ)
        return false;

    return !m_DynamicColliders[ToIndex(iX, iY, iZ)].empty();
}

_uint CUniform_Grid::Get_DynamicColliderCount(int iX, int iY, int iZ) const
{
    if (iX < 0 || iX >= m_iGridDimX ||
        iY < 0 || iY >= m_iGridDimY ||
        iZ < 0 || iZ >= m_iGridDimZ)
        return 0;

    return static_cast<_uint>(m_DynamicColliders[ToIndex(iX, iY, iZ)].size());
}

std::unique_ptr<CUniform_Grid> CUniform_Grid::Create(const _float3& vWorldMin, const _float3& vWorldMax, _float fCellSize)
{
    std::unique_ptr<CUniform_Grid> pInstance = std::make_unique<CUniform_Grid>();

    if (FAILED(pInstance->Initialize(vWorldMin, vWorldMax, fCellSize)))
        return nullptr;

    return pInstance;
}

NS_END
