#pragma once

#include "Engine_Define.h"
#include "Collider.h"

NS_BEGIN(Engine)

class ENGINE_DLL CDebug_Renderer final
{
public:
    CDebug_Renderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    ~CDebug_Renderer();

public:
    HRESULT Initialize();

    void Begin();
    void End();

    void Draw_Collider(const COLLIDER_PROXY_DATA& tProxy);

private:
    void Draw_Box(const COLLIDER_PROXY_DATA& tProxy);
    void Draw_Sphere(const COLLIDER_PROXY_DATA& tProxy);
    void Draw_Plane(const COLLIDER_PROXY_DATA& tProxy);
    void Draw_AABB(const AABB& aabb);

private:
    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pContext = nullptr;

    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_pBatch;
    std::unique_ptr<DirectX::BasicEffect> m_pEffect;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout;

    _bool m_bBegun = false;

public:
    static std::unique_ptr<CDebug_Renderer> Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
};

NS_END
