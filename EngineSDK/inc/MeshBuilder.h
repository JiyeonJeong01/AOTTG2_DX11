#pragma once

#include "Converter_Struct.h"
#include "Engine_Define.h"
#include "Mesh.h"

NS_BEGIN(Engine)

class ENGINE_DLL CMeshBuilder final
{
public :
    static HRESULT  Create_Builtin(ID3D11Device* pDevice, MESH_ENTRY& outEntry, const ASSET_GUID& tGUID);

    static HRESULT  Create_Mesh(ID3D11Device* pDevice, const MESH_DESC& tDesc, MESH_ENTRY& outEntry);
    static HRESULT  Create_Mesh_By_Geometry(ID3D11Device* pDevice, Geometry eGeometry, MESH_ENTRY& outEntry);
    static HRESULT  Create_Rect_VtxTex(ID3D11Device* pDevice, MESH_ENTRY& outEntry);
    static HRESULT  Create_Rect_VtxNorTex(ID3D11Device* pDevice, MESH_ENTRY& outEntry);
    static HRESULT  Create_Cube_VtxCol(ID3D11Device* pDevice, MESH_ENTRY& outEntry);
    static HRESULT  Create_Sphere_VtxCol(ID3D11Device* pDevice, MESH_ENTRY& outEntry, _uint iStack = 5, _uint iSlice = 10, _float fRadius = 0.5f);
    static HRESULT  Create_RibbonLine_VtxCol(ID3D11Device* pDevice, MESH_ENTRY& outEntry, _uint iNumCnt);

    static HRESULT Load_ModelDesc(const std::filesystem::path& modelPath, MODEL_DESC& outDesc);
private :
    static _bool Split_KeyValue(const std::string& line, std::string& outKey, std::string& outValue);

};

#define HANDLE_FAIL

NS_END
