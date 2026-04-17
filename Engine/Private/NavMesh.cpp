#include "NavMesh.h"

#include "NavBuilder.h"
#include "Debug_Renderer.h"
#include "Editor_System.h"

CNavMesh::CNavMesh()
{
}

CNavMesh::~CNavMesh()
{
}

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

    return fDistSq <= (0.4f * 0.4f);
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

void CNavMesh::Debug_Render(CDebug_Renderer* pDebugRenderer) const
{
    if (!pDebugRenderer)
        return;

    _int iSize = (_int)m_vecWayPoints.size() - 1;
    if (iSize < 0)
        iSize = 0;

    for (_int i = 0; i < iSize; ++i)
    {
        _float3 vPoint = m_vecWayPoints[i];
        _float3 vNextPoint = m_vecWayPoints[i + 1];
        vPoint.y += 0.4f;

        pDebugRenderer->Draw_Line(vPoint, vNextPoint, XMVECTOR{ 1.f, 0.f, 0.f, 1.f });
    }
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

    std::vector<_float3> vecRawWayPoints;

    if (m_vecPathCells.size() == 1)
    {
        vecRawWayPoints.push_back(m_vTargetPos);
    }
    else
    {
        for (_uint i = 0; i + 1 < m_vecPathCells.size(); ++i)
        {
            const _int iCellIndexA = m_vecPathCells[i];
            const _int iCellIndexB = m_vecPathCells[i + 1];

            const _float3 vMidPoint = Get_SharedEdgeMidPoint(iCellIndexA, iCellIndexB);

            if (!vecRawWayPoints.empty())
            {
                const _float3& vPrev = vecRawWayPoints.back();

                const _float fDx = vMidPoint.x - vPrev.x;
                const _float fDz = vMidPoint.z - vPrev.z;
                const _float fDistSq = fDx * fDx + fDz * fDz;

                if (fDistSq < 0.0001f)
                    continue;
            }

            vecRawWayPoints.push_back(vMidPoint);
        }

        if (!vecRawWayPoints.empty())
        {
            const _float3& vPrev = vecRawWayPoints.back();

            const _float fDx = m_vTargetPos.x - vPrev.x;
            const _float fDz = m_vTargetPos.z - vPrev.z;
            const _float fDistSq = fDx * fDx + fDz * fDz;

            if (fDistSq >= 0.0001f)
                vecRawWayPoints.push_back(m_vTargetPos);
        }
        else
        {
            vecRawWayPoints.push_back(m_vTargetPos);
        }
    }

    if (vecRawWayPoints.size() <= 2)
    {
        m_vecWayPoints = vecRawWayPoints;
        return;
    }

    m_vecWayPoints.push_back(vecRawWayPoints[0]);

    for (_uint i = 1; i + 1 < vecRawWayPoints.size(); ++i)
    {
        const _float3& vPrev = m_vecWayPoints.back();
        const _float3& vCur = vecRawWayPoints[i];
        const _float3& vNext = vecRawWayPoints[i + 1];

        _float fAX = vCur.x - vPrev.x;
        _float fAZ = vCur.z - vPrev.z;
        _float fALen = sqrtf(fAX * fAX + fAZ * fAZ);

        _float fBX = vNext.x - vCur.x;
        _float fBZ = vNext.z - vCur.z;
        _float fBLen = sqrtf(fBX * fBX + fBZ * fBZ);

        if (fALen <= 0.0001f || fBLen <= 0.0001f)
            continue;

        fAX /= fALen;
        fAZ /= fALen;
        fBX /= fBLen;
        fBZ /= fBLen;

        const _float fDot = fAX * fBX + fAZ * fBZ;

        /* 거의 같은 방향이면 가운데 점 제거 */
        if (fDot >= 0.98f)
            continue;

        m_vecWayPoints.push_back(vCur);
    }

    m_vecWayPoints.push_back(vecRawWayPoints.back());
}

_float3 CNavMesh::Get_Dir(const _float3& vCurPos)
{
    if (false == m_bHasTarget)
        return _float3(0.f, 0.f, 0.f);

    if (m_iWayPointIndex >= m_vecWayPoints.size())
        return _float3(0.f, 0.f, 0.f);

    const _float fReachDist = 0.4f * 0.4f;

    while (m_iWayPointIndex < m_vecWayPoints.size())
    {
        _float3 vTarget = m_vecWayPoints[m_iWayPointIndex];

        _float fX = vTarget.x - vCurPos.x;
        _float fZ = vTarget.z - vCurPos.z;

        const _float fDistSq = fX * fX + fZ * fZ;

        _bool bAdvance = false;

        /* 가까이 왔으면 다음 waypoint로 */
        if (fDistSq <= fReachDist)
        {
            bAdvance = true;
        }
        /* waypoint를 이미 지나쳤으면 다음 waypoint로 */
        else if (m_iWayPointIndex > 0)
        {
            const _float3& vPrevTarget = m_vecWayPoints[m_iWayPointIndex - 1];

            const _float fSegX = vTarget.x - vPrevTarget.x;
            const _float fSegZ = vTarget.z - vPrevTarget.z;

            const _float fToCurX = vCurPos.x - vTarget.x;
            const _float fToCurZ = vCurPos.z - vTarget.z;

            const _float fDot = fSegX * fToCurX + fSegZ * fToCurZ;

            if (fDot > 0.f)
                bAdvance = true;
        }

        if (false == bAdvance)
            break;

        ++m_iWayPointIndex;
    }

    if (m_iWayPointIndex >= m_vecWayPoints.size())
    {
        Clear_Target();
        return _float3(0.f, 0.f, 0.f);
    }

    const _float3& vTarget = m_vecWayPoints[m_iWayPointIndex];

    const _float fX = vTarget.x - vCurPos.x;
    const _float fZ = vTarget.z - vCurPos.z;

    const _float fLen = sqrtf(fX * fX + fZ * fZ);
    if (fLen <= 0.0001f)
        return _float3(0.f, 0.f, 0.f);

    return _float3(fX / fLen, 0.f, fZ / fLen);
}

_bool CNavMesh::Can_Advance_WayPoint(const _float3& vCurPos) const
{
    if (m_iWayPointIndex >= To<_uint>(m_vecWayPoints.size()))
        return false;

    const _float3& vWayPoint = m_vecWayPoints[m_iWayPointIndex];

    const _float fReachDistSq = 0.5f * 0.5f;

    const _float fDx = vWayPoint.x - vCurPos.x;
    const _float fDz = vWayPoint.z - vCurPos.z;
    const _float fDistSq = fDx * fDx + fDz * fDz;

    if (fDistSq <= fReachDistSq)
        return true;

    if (m_iWayPointIndex == 0)
        return false;

    const _float3& vPrevWayPoint = m_vecWayPoints[m_iWayPointIndex - 1];

    const _float fSegX = vWayPoint.x - vPrevWayPoint.x;
    const _float fSegZ = vWayPoint.z - vPrevWayPoint.z;

    const _float fToCurX = vCurPos.x - vWayPoint.x;
    const _float fToCurZ = vCurPos.z - vWayPoint.z;

    const _float fDot = fSegX * fToCurX + fSegZ * fToCurZ;

    return fDot > 0.f;
}

_float3 CNavMesh::Get_SharedEdgeMidPoint(_int iCellIndexA, _int iCellIndexB) const
{
    const NAV_CELL& tCellA = m_vecNavCells[iCellIndexA];
    const NAV_CELL& tCellB = m_vecNavCells[iCellIndexB];

    _int iSharedPointIndices[2] = { -1, -1 };
    _int iSharedCount = 0;

    for (_int i = 0; i < 3; ++i)
    {
        const _int iPointIndexA = tCellA.iPoints[i];

        for (_int j = 0; j < 3; ++j)
        {
            const _int iPointIndexB = tCellB.iPoints[j];

            if (iPointIndexA == iPointIndexB)
            {
                if (iSharedCount < 2)
                    iSharedPointIndices[iSharedCount] = iPointIndexA;

                ++iSharedCount;
                break;
            }
        }
    }

    if (iSharedCount < 2)
        return Get_CellCenter(iCellIndexB);

    const _float3& vPoint0 = m_vecNavPoints[iSharedPointIndices[0]].vPos;
    const _float3& vPoint1 = m_vecNavPoints[iSharedPointIndices[1]].vPos;

    return _float3(
        (vPoint0.x + vPoint1.x) * 0.5f,
        (vPoint0.y + vPoint1.y) * 0.5f,
        (vPoint0.z + vPoint1.z) * 0.5f
    );
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
