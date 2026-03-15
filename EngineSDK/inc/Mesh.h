#pragma once

#include "Engine_Define.h"
#include "Engine_Log.h"
#include "Identity.h"

NS_BEGIN(Engine)
    typedef struct ENGINE_DLL tagMeshDesc
{
    const void* pVertices{ };
    _uint       iVertexStride{ };
    _uint       iVertexCnt{ };

    const void* pIndices{ };
    _uint       iIndexStride{ };
    _uint       iIndexCnt{ };

    DXGI_FORMAT eIndexFormat = DXGI_FORMAT_R16_UINT;
    D3D11_PRIMITIVE_TOPOLOGY eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}MESH_DESC;

typedef struct ENGINE_DLL tagMeshEntry 
{
    ASSET_GUID  tGUID{};

    Microsoft::WRL::ComPtr<ID3D11Buffer> pVB{ };
    Microsoft::WRL::ComPtr<ID3D11Buffer> pIB{ };

    _uint                       iVertexStride = 0;
    _uint                       iVertexCount = 0; /* optional */

    DXGI_FORMAT                 eIndexFormat = DXGI_FORMAT_R16_UINT;
    _uint                       iIndexCount = 0;

    D3D11_PRIMITIVE_TOPOLOGY    eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    _uint                       iVBOffset = 0; /* usually 0 */

    _float3                     minAABB{}, maxAABB{};

public:
    _bool Is_Valid() const noexcept
    {
        return (pVB != nullptr) && (pIB != nullptr) && (iVertexStride != 0) && (iIndexCount != 0);
    }

    void Bind_IA(ID3D11DeviceContext* pCtx, UINT iStartSlot = 0) const
    {
#ifdef _DEBUG
        /* TODO : 안정화되면 enum으로 생성하게 바꾸자. 매 프레임 이거 검사하는 건 좀 에바. */
        if (eIndexFormat != DXGI_FORMAT_R16_UINT && eIndexFormat != DXGI_FORMAT_R32_UINT)
        {
            _DEBUG_ERROR_BREAK("Invalid index format! Only R16/R32 allowed.");
        }
#endif
        if (!pCtx)
            return;
        ID3D11Buffer* vbs[] = { pVB.Get() };
        UINT iStrides[] = { (UINT)iVertexStride };
        UINT iOffsets[] = { (UINT)iVBOffset };
        pCtx->IASetVertexBuffers(iStartSlot, 1, vbs, iStrides, iOffsets);
        pCtx->IASetIndexBuffer(pIB.Get(), eIndexFormat, 0);
        pCtx->IASetPrimitiveTopology(eTopology);
    }

    /* For submesh... */
    void Draw(ID3D11DeviceContext* pCtx, _uint firstIndex = 0, _uint indexCount = 0) const
    {
        if (!pCtx)
            return;

        const UINT cnt = (indexCount == 0) ? (UINT)iIndexCount : (UINT)indexCount;
        pCtx->DrawIndexed(cnt, (UINT)firstIndex, 0);
    }

    void Draw_Instanced(ID3D11DeviceContext* pCtx, UINT instanceCount, _uint firstIndex = 0, _uint indexCount = 0) const
    {
        if (!pCtx || instanceCount == 0) return;
        const UINT cnt = (indexCount == 0) ? (UINT)iIndexCount : (UINT)indexCount;
        pCtx->DrawIndexedInstanced(cnt, instanceCount, (UINT)firstIndex, 0, 0);
    }

    void Get_Mesh_LocalAABB(_float3& outMin, _float3& outMax) const
    {
        outMin = minAABB;
        outMax = maxAABB;
    }

}MESH_ENTRY;

NS_END
