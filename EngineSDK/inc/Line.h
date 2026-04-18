#pragma once

#include "Engine_Define.h"
#include "Component_Struct.h"

NS_BEGIN(Engine)

typedef struct tagLinePoint
{
    _float3 vPosition{};
    _float3 vRight{};
    _float  fWidth = 1.f;
    _float4 vColor = { 1.f, 1.f, 1.f, 1.f };
} LINE_POINT;

class ENGINE_DLL CLine final
{
public :
    CLine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    ~CLine();


private :
    ID3D11Device*           m_pDevice{};
    ID3D11DeviceContext*    m_pContext{};

public :
    uint32_t    m_hMesh = INVALID_HANDLE_UINT;
    uint32_t    m_hShader = INVALID_HANDLE_UINT;
    _float      m_fThickness = 0.5f;
    _float4     m_vColor = { 0.f, 0.f, 0.f, 1.f };
    _uint       m_iMaxPoints = {};
    _uint       m_iCurPoints = {};

public :
    HRESULT Initialize(_uint iMaxPoints, _float fThickness);
    HRESULT     Update(const _float3* pPoints, _uint iNumPoints);
    void        Submit();


    HRESULT Initialize_Trail(_uint iMaxPoints, _float fThickness);
    HRESULT Update_Trail(const LINE_POINT* pPoints, _uint iNumPoints);
    void    Submit_Trail();

public :
    static std::unique_ptr<CLine> Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
                                            _uint iNumPoints, _float fThickness, LINE_TYPE eType);
};



NS_END
