#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

typedef struct tagNavPoint
{
    _float3 vPos = {};
} NAV_POINT;

typedef struct tagNavCell
{
    _int iPoints[3] = { -1, -1, -1 };
    _int iNeighbors[3] = { -1, -1, -1 };
    _int iIndex = -1;

    tagNavCell() = default;

    tagNavCell(_int iPointA, _int iPointB, _int iPointC, _int iCellIndex)
        : iIndex(iCellIndex)
    {
        iPoints[0] = iPointA;
        iPoints[1] = iPointB;
        iPoints[2] = iPointC;
    }
} NAV_CELL;

typedef struct tagNavPointFile
{
    _float3 vPos = {};
} NAV_POINT_FILE;

typedef struct tagNavCellFile
{
    _int iPoints[3] = { -1, -1, -1 };
    _int iIndex = -1;
} NAV_CELL_FILE;

typedef struct tagNavFileHeader
{
    _uint iMagic = 0x3141564E;   // 'NVA1'
    _uint iVersion = 1;
    _uint iPointCount = 0;
    _uint iCellCount = 0;
} NAV_FILE_HEADER;

typedef struct tagNavRuntimeData
{
    std::vector<NAV_POINT> vecNavPoints;
    std::vector<NAV_CELL> vecNavCells;
} NAV_RUNTIME_DATA;

NS_END
