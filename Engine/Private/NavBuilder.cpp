#include "NavBuilder.h"

_bool CNavBuilder::Is_SameEdge(_int a0, _int a1, _int b0, _int b1)
{
    return (a0 == b0 && a1 == b1) || (a0 == b1 && a1 == b0);
}

_bool CNavBuilder::Find_SharedEdge(const NAV_CELL& src, const NAV_CELL& dst, _int& iOutSrcEdge, _int& iOutDstEdge)
{
    for (_int i = 0; i < 3; ++i)
    {
        const _int iSrcA = src.iPoints[i];
        const _int iSrcB = src.iPoints[(i + 1) % 3];

        for (_int j = 0; j < 3; ++j)
        {
            const _int iDstA = dst.iPoints[j];
            const _int iDstB = dst.iPoints[(j + 1) % 3];

            if (Is_SameEdge(iSrcA, iSrcB, iDstA, iDstB))
            {
                iOutSrcEdge = i;
                iOutDstEdge = j;
                return true;
            }
        }
    }

    iOutSrcEdge = -1;
    iOutDstEdge = -1;
    return false;
}

void CNavBuilder::Build_NavCellNeighbors(const std::vector<NAV_CELL>& inCells, std::vector<NAV_CELL>& outCells)
{
    outCells = inCells;

    for (_uint i = 0; i < outCells.size(); ++i)
    {
        outCells[i].iIndex = (_int)i;
        outCells[i].iNeighbors[0] = -1;
        outCells[i].iNeighbors[1] = -1;
        outCells[i].iNeighbors[2] = -1;
    }

    for (_uint i = 0; i < outCells.size(); ++i)
    {
        for (_uint j = i + 1; j < outCells.size(); ++j)
        {
            _int iSrcEdge = -1;
            _int iDstEdge = -1;

            if (Find_SharedEdge(outCells[i], outCells[j], iSrcEdge, iDstEdge))
            {
                outCells[i].iNeighbors[iSrcEdge] = (_int)j;
                outCells[j].iNeighbors[iDstEdge] = (_int)i;
            }
        }
    }
}

_bool CNavBuilder::Save_Nav(const wchar_t* pFilePath, const std::vector<NAV_POINT>& vecNavPoints, const std::vector<NAV_CELL>& vecBuiltCells)
{
    if (pFilePath == nullptr)
        return false;

    std::filesystem::path path(pFilePath);
    std::filesystem::path dir = path.parent_path();

    if (!dir.empty() && !std::filesystem::exists(dir)) {
        if (!std::filesystem::create_directories(dir)) {
            return false;
        }
    }

    HANDLE hFile = CreateFileW(
        pFilePath,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    DWORD dwByte = 0;

    NAV_FILE_HEADER tHeader{};
    tHeader.iPointCount = (_uint)vecNavPoints.size();
    tHeader.iCellCount = (_uint)vecBuiltCells.size();

    if (FALSE == WriteFile(hFile, &tHeader, sizeof(NAV_FILE_HEADER), &dwByte, nullptr) ||
        dwByte != sizeof(NAV_FILE_HEADER))
    {
        CloseHandle(hFile);
        return false;
    }

    if (tHeader.iPointCount > 0)
    {
        const DWORD dwPointBytes = (DWORD)(sizeof(NAV_POINT) * tHeader.iPointCount);

        if (FALSE == WriteFile(hFile, vecNavPoints.data(), dwPointBytes, &dwByte, nullptr) ||
            dwByte != dwPointBytes)
        {
            CloseHandle(hFile);
            return false;
        }
    }

    if (tHeader.iCellCount > 0)
    {
        const DWORD dwCellBytes = (DWORD)(sizeof(NAV_CELL) * tHeader.iCellCount);

        if (FALSE == WriteFile(hFile, vecBuiltCells.data(), dwCellBytes, &dwByte, nullptr) ||
            dwByte != dwCellBytes)
        {
            CloseHandle(hFile);
            return false;
        }
    }

    CloseHandle(hFile);
    return true;
}

_bool CNavBuilder::Load_Nav(const wchar_t* pFilePath, std::vector<NAV_POINT>& vecNavPoints, std::vector<NAV_CELL>& vecNavCells)
{
    if (pFilePath == nullptr)
        return false;

    HANDLE hFile = CreateFileW(
        pFilePath,
        GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    DWORD dwByte = 0;
    NAV_FILE_HEADER tHeader{};

    if (FALSE == ReadFile(hFile, &tHeader, sizeof(NAV_FILE_HEADER), &dwByte, nullptr) ||
        dwByte != sizeof(NAV_FILE_HEADER))
    {
        CloseHandle(hFile);
        return false;
    }

    if (tHeader.iMagic != 0x3141564E || tHeader.iVersion != 1)
    {
        CloseHandle(hFile);
        return false;
    }

    std::vector<NAV_POINT> vecLoadedPoints;
    std::vector<NAV_CELL> vecLoadedCells;

    if (tHeader.iPointCount > 0)
    {
        vecLoadedPoints.resize(tHeader.iPointCount);

        const DWORD dwPointBytes = (DWORD)(sizeof(NAV_POINT) * tHeader.iPointCount);

        if (FALSE == ReadFile(hFile, vecLoadedPoints.data(), dwPointBytes, &dwByte, nullptr) ||
            dwByte != dwPointBytes)
        {
            CloseHandle(hFile);
            return false;
        }
    }

    if (tHeader.iCellCount > 0)
    {
        vecLoadedCells.resize(tHeader.iCellCount);

        const DWORD dwCellBytes = (DWORD)(sizeof(NAV_CELL) * tHeader.iCellCount);

        if (FALSE == ReadFile(hFile, vecLoadedCells.data(), dwCellBytes, &dwByte, nullptr) ||
            dwByte != dwCellBytes)
        {
            CloseHandle(hFile);
            return false;
        }
    }

    CloseHandle(hFile);

    for (_uint i = 0; i < (_uint)vecLoadedCells.size(); ++i)
    {
        vecLoadedCells[i].iIndex = (_int)i;
    }

    vecNavPoints = std::move(vecLoadedPoints);
    vecNavCells = std::move(vecLoadedCells);

    return true;
}

