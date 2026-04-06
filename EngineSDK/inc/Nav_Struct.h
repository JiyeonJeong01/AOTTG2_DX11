#pragma once
#include "Engine_Define.h"

typedef struct tagNavPoint
{
    _float3 vPos = {};
} NAV_POINT;

typedef struct tagNavCell
{
    _int iPoints[3] = { -1, -1, -1 };
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
