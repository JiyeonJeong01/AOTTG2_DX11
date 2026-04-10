#include "Nav_Struct.h"

typedef struct tagAStarNode
{
    _float fG = 0.f;
    _float fH = 0.f;
    _float fF = 0.f;
    _int iParent = -1;
    _bool bClosed = false;
    _bool bOpened = false;
} ASTAR_NODE;

NS_BEGIN(Engine)

class ENGINE_DLL CNavMesh
{
public:
    _bool       Set_TargetPosition(const _float3& vCurPos, const _float3& vTargetPos);
    _float3     Get_Dir(const _float3& vCurPos);

    _bool       Is_Arrived(const _float3& vCurPos) const;
    void        Clear_Target();

    _bool       Load(const wchar_t* pFilePath);

private:
    /* 인덱스 버퍼 구조 */
    std::vector<NAV_POINT> m_vecNavPoints;
    std::vector<NAV_CELL>  m_vecNavCells;

    _int m_iCurrentCellIndex = -1;
    _int m_iTargetCellIndex = -1;

    _float3                 m_vTargetPos = {};
    std::vector<_int>       m_vecPathCells; /* A* 결과 (셀 단위 경로) */
    std::vector<_float3>    m_vecWayPoints; /* 실제 이동 좌표 리스트 */
    _uint m_iWayPointIndex = 0;
    _bool m_bHasTarget = false;

private:
    _int        Find_CellIndex(const _float3& vPos) const;
    _float3     Get_CellCenter(_int iCellIndex) const;

    _bool       Build_Path_AStar(_int iStartCellIndex, _int iGoalCellIndex);
    void        Build_WayPoints();

    _bool Point_In_TriangleXZ(const _float3& vPoint, const _float3& vA, const _float3& vB, const _float3& vC) const;
};

NS_END
