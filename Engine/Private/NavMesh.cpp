#include "NavMesh.h"

#include "NavBuilder.h"

_bool CNavMesh::Set_TargetPosition(const _float3& vCurPos, const _float3& vTargetPos)
{
    m_iCurrentCellIndex = Find_CellIndex(vCurPos);
    m_iTargetCellIndex = Find_CellIndex(vTargetPos);

    if (m_iCurrentCellIndex < 0 || m_iTargetCellIndex < 0)
    {
        Clear_Target();
        return false;
    }

    m_vTargetPos = vTargetPos;
    m_vecPathCells.clear();
    m_vecWayPoints.clear();
    m_iWayPointIndex = 0;

    /* 이미 해당 셀이므로, 타겟에게 직진 */
    if (m_iCurrentCellIndex == m_iTargetCellIndex)
    {
        m_vecWayPoints.push_back(vTargetPos);
        m_bHasTarget = true;
        return true;
    }

    /* A*로 경로 찾아오기 */
    if (false == Build_Path_AStar(m_iCurrentCellIndex, m_iTargetCellIndex))
    {
        Clear_Target();
        return false;
    }

    Build_WayPoints();
    m_bHasTarget = !m_vecWayPoints.empty();
    return m_bHasTarget;
}

_bool CNavMesh::Is_Arrived(const _float3& vCurPos) const
{
    if (false == m_bHasTarget)
        return true;

    const _float fX = m_vTargetPos.x - vCurPos.x;
    const _float fZ = m_vTargetPos.z - vCurPos.z;
    const _float fDistSq = fX * fX + fZ * fZ;

    return fDistSq <= (0.15f * 0.15f);
}

void CNavMesh::Clear_Target()
{
    m_iTargetCellIndex = -1;
    m_vTargetPos = _float3{};
    m_vecPathCells.clear();
    m_vecWayPoints.clear();
    m_iWayPointIndex = 0;
    m_bHasTarget = false;
}

_bool CNavMesh::Load(const wchar_t* pFilePath)
{
    m_vecNavPoints.clear();
    m_vecNavCells.clear();

    if (false == CNavBuilder::Load_Nav(pFilePath, m_vecNavPoints, m_vecNavCells))
        return false;

    return true;
}

_int CNavMesh::Find_CellIndex(const _float3& vPos) const
{
    for (_uint i = 0; i < m_vecNavCells.size(); ++i)
    {
        const NAV_CELL& tCell = m_vecNavCells[i];

        const _int iPointA = tCell.iPoints[0];
        const _int iPointB = tCell.iPoints[1];
        const _int iPointC = tCell.iPoints[2];

        if (iPointA < 0 || iPointB < 0 || iPointC < 0)
            continue;

        if (iPointA >= (_int)m_vecNavPoints.size() ||
            iPointB >= (_int)m_vecNavPoints.size() ||
            iPointC >= (_int)m_vecNavPoints.size())
            continue;

        const _float3& vA = m_vecNavPoints[iPointA].vPos;
        const _float3& vB = m_vecNavPoints[iPointB].vPos;
        const _float3& vC = m_vecNavPoints[iPointC].vPos;

        if (Point_In_TriangleXZ(vPos, vA, vB, vC))
            return (_int)i;
    }

    return -1;
}

/* 셀 기준으로 만들어진 경로를 각 셀의 중심점 기준으로 바꾼다 */
_float3 CNavMesh::Get_CellCenter(_int iCellIndex) const
{
    const NAV_CELL& tCell = m_vecNavCells[iCellIndex];

    const _float3& vA = m_vecNavPoints[tCell.iPoints[0]].vPos;
    const _float3& vB = m_vecNavPoints[tCell.iPoints[1]].vPos;
    const _float3& vC = m_vecNavPoints[tCell.iPoints[2]].vPos;

    return _float3(
        (vA.x + vB.x + vC.x) / 3.f,
        (vA.y + vB.y + vC.y) / 3.f,
        (vA.z + vB.z + vC.z) / 3.f
    );
}

_bool CNavMesh::Build_Path_AStar(_int iStartCellIndex, _int iGoalCellIndex)
{
    if (iStartCellIndex < 0 || iGoalCellIndex < 0)
        return false;

    /* 셀 개수만큼 ASTAR_NODE 생성하기 */
    std::vector<ASTAR_NODE> vecNodes(m_vecNavCells.size());

    auto Heuristic = [&](int iFrom, int iTo) -> _float
        {
            const _float3 vFrom = Get_CellCenter(iFrom);
            const _float3 vTo = Get_CellCenter(iTo);

            const _float fX = vTo.x - vFrom.x;
            const _float fZ = vTo.z - vFrom.z;
            return sqrtf(fX * fX + fZ * fZ);
        };

    std::vector<_int> vecOpen;
    vecOpen.push_back(iStartCellIndex);

    vecNodes[iStartCellIndex].fG = 0.f;                                                     /* G : 시작점 ~ 현재 노드까지 걸린 실제 비용 */
    vecNodes[iStartCellIndex].fH = Heuristic(iStartCellIndex, iGoalCellIndex);      /* H : 현재 노드 ~ 도착점까지 예상되는 비용 */
    vecNodes[iStartCellIndex].fF = vecNodes[iStartCellIndex].fH;                            /* F : G + H */
    vecNodes[iStartCellIndex].iParent = -1;
    vecNodes[iStartCellIndex].bOpened = true;

    while (false == vecOpen.empty())
    {
        /* 가장 좋은 후보 찾기 */
        _int iBestOpenIndex = 0;
        _int iCurrentCellIndex = vecOpen[0];

        for (_uint i = 1; i < vecOpen.size(); ++i)
        {
            const _int iCellIndex = vecOpen[i];
            /* 1. F 값이 가장 작은 셀 */
            if (vecNodes[iCellIndex].fF < vecNodes[iCurrentCellIndex].fF)
            {
                iCurrentCellIndex = iCellIndex;
                iBestOpenIndex = To<_int>(i);
            }
        }

        vecOpen.erase(vecOpen.begin() + iBestOpenIndex);
        vecNodes[iCurrentCellIndex].bClosed = true;

        /* 도착 */
        if (iCurrentCellIndex == iGoalCellIndex)
        {
            m_vecPathCells.clear();

            _int iTraceCellIndex = iGoalCellIndex;
            while (iTraceCellIndex != -1)
            {
                m_vecPathCells.push_back(iTraceCellIndex);
                iTraceCellIndex = vecNodes[iTraceCellIndex].iParent;
            }

            std::reverse(m_vecPathCells.begin(), m_vecPathCells.end());
            return true;
        }

        /* 이웃 탐색 */
        const NAV_CELL& tCell = m_vecNavCells[iCurrentCellIndex];

        for (_int i = 0; i < 3; ++i)
        {
            const _int iNeighborCellIndex = tCell.iNeighbors[i];
            if (iNeighborCellIndex < 0)
                continue;

            if (vecNodes[iNeighborCellIndex].bClosed)
                continue;

            const _float fNewG =
                vecNodes[iCurrentCellIndex].fG + Heuristic(iCurrentCellIndex, iNeighborCellIndex);

            /* 비용 계산 */
            if (false == vecNodes[iNeighborCellIndex].bOpened || fNewG < vecNodes[iNeighborCellIndex].fG)
            {
                vecNodes[iNeighborCellIndex].fG = fNewG;
                vecNodes[iNeighborCellIndex].fH = Heuristic(iNeighborCellIndex, iGoalCellIndex);
                vecNodes[iNeighborCellIndex].fF = vecNodes[iNeighborCellIndex].fG + vecNodes[iNeighborCellIndex].fH;
                vecNodes[iNeighborCellIndex].iParent = iCurrentCellIndex; /* 내 인덱스로 부모 인덱스 갱신 */

                /* 새로 발견한 셀 */
                if (false == vecNodes[iNeighborCellIndex].bOpened)
                {
                    vecNodes[iNeighborCellIndex].bOpened = true;
                    vecOpen.push_back(iNeighborCellIndex); 
                }
            }
        }
    }

    return false;
}

void CNavMesh::Build_WayPoints()
{
    m_vecWayPoints.clear();
    m_iWayPointIndex = 0;

    if (m_vecPathCells.empty())
        return;

    for (_uint i = 1; i + 1 < m_vecPathCells.size(); ++i)
    {
        const _int iCellIndex = m_vecPathCells[i];
        m_vecWayPoints.push_back(Get_CellCenter(iCellIndex));
    }

    m_vecWayPoints.push_back(m_vTargetPos);
}

_float3 CNavMesh::Get_Dir(const _float3& vCurPos)
{
    if (false == m_bHasTarget)
        return _float3(0.f, 0.f, 0.f);

    if (m_iWayPointIndex >= m_vecWayPoints.size())
        return _float3(0.f, 0.f, 0.f);

    _float3 vTarget = m_vecWayPoints[m_iWayPointIndex];

    _float fX = vTarget.x - vCurPos.x;
    _float fZ = vTarget.z - vCurPos.z;

    const _float fDistSq = fX * fX + fZ * fZ;
    const _float fReachDist = 0.15f * 0.15f;

    if (fDistSq <= fReachDist)
    {
        ++m_iWayPointIndex;

        if (m_iWayPointIndex >= m_vecWayPoints.size())
        {
            Clear_Target();
            return _float3(0.f, 0.f, 0.f);
        }

        vTarget = m_vecWayPoints[m_iWayPointIndex];
        fX = vTarget.x - vCurPos.x;
        fZ = vTarget.z - vCurPos.z;
    }

    const _float fLen = sqrtf(fX * fX + fZ * fZ);
    if (fLen <= 0.0001f)
        return _float3(0.f, 0.f, 0.f);

    /* 정규화 후 리턴 */
    return _float3(fX / fLen, 0.f, fZ / fLen);
}

_bool CNavMesh::Point_In_TriangleXZ(const _float3& vPoint, const _float3& vA, const _float3& vB, const _float3& vC) const
{
    auto SignXZ = [](const _float3& v1, const _float3& v2, const _float3& v3) -> _float
        {
            return (v1.x - v3.x) * (v2.z - v3.z) - (v2.x - v3.x) * (v1.z - v3.z);
        };

    const _float fD1 = SignXZ(vPoint, vA, vB);
    const _float fD2 = SignXZ(vPoint, vB, vC);
    const _float fD3 = SignXZ(vPoint, vC, vA);

    const _bool bHasNeg = (fD1 < 0.f) || (fD2 < 0.f) || (fD3 < 0.f);
    const _bool bHasPos = (fD1 > 0.f) || (fD2 > 0.f) || (fD3 > 0.f);

    return !(bHasNeg && bHasPos);
}
