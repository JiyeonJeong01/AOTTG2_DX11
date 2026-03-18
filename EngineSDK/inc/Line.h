#pragma once

#include "Engine_Define.h"
#include "Component_Struct.h"

NS_BEGIN(Engine)

class ENGINE_DLL CLine final
{
public :
    CLine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    ~CLine();

    HRESULT Initialize(_uint iMaxPoints, _float fThickness);

private :
    ID3D11Device*           m_pDevice{};
    ID3D11DeviceContext*    m_pContext{};

public :
    uint32_t    m_hMesh = INVALID_HANDLE_UINT;
    _float      m_fThickness = 0.5f;
    _float4     m_vColor = { 0.f, 0.f, 0.f, 1.f };
    _uint       m_iMaxPoints = {};
    _uint       m_iCurPoints = {};

public :
    HRESULT     Update(const _float3* pPoints, _uint iNumPoints);
    void        Submit();

public :
    static std::unique_ptr<CLine> Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
                                            _uint iNumPoints, _float fThickness);
};


NS_END
