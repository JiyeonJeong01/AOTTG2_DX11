#pragma once
#include "Nav_Struct.h"

class CNavBuilder
{
public :
    static _bool    Is_SameEdge(_int a0, _int a1, _int b0, _int b1);
    static _bool    Find_SharedEdge(const NAV_CELL& src, const NAV_CELL& dst, _int& iOutSrcEdge, _int& iOutDstEdge);
    static void     Build_NavCellNeighbors(const std::vector<NAV_CELL>& inCells, std::vector<NAV_CELL>& outCells);
    static _bool    Save_Nav(const wchar_t* pFilePath, const std::vector<NAV_POINT>& vecNavPoints, const std::vector<NAV_CELL>& vecBuiltCells);
    static _bool    Load_Nav(const wchar_t* pFilePath, std::vector<NAV_POINT>& vecNavPoints, std::vector<NAV_CELL>& vecNavCells);
};

