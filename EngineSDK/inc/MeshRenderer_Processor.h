#pragma once
#include "Render_Struct.h"
#include "MeshRenderer.h"
#include "Component_Processor_Impl.h"

NS_BEGIN(Engine)

class CTransform_Processor;
class CRenderer_System;

class ENGINE_DLL CMeshRenderer_Processor final : public CComponent_Processor_Impl<CMeshRenderer, COMPONENT_TYPE::MESH_RENDERER>
{
public :
    CMeshRenderer_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CTransform_Processor* pTransformProcessor);
    ~CMeshRenderer_Processor() override;
public:
    HRESULT Initialize() override;
    void Update(_float fDT) override;
    void LateUpdate(_float fDT) override;
    void Render();

    void Begin_Frame();
    void End_Frame();
    void Build_Queue(std::vector<DRAW_CMD>& outCmds);

    HRESULT Initialize_From_Spec_Impl(COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec) override;

private :
    void Initialize_Component_Data(COMPONENT_HANDLE hComponent) override;

private:
    uint64_t Make_SortKey(const MESH_RENDERER_DATA& d) const;
    void Execute_Draw(const DRAW_CMD& cmd);

private :
    ID3D11Device*               m_pDevice{};
    ID3D11DeviceContext*        m_pContext{};
    CTransform_Processor*       m_pTransformProcessor{};

public:
    static std::unique_ptr<CMeshRenderer_Processor> Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CTransform_Processor* pTransform);
};

NS_END
