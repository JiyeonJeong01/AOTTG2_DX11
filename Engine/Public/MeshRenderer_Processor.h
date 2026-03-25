#pragma once
#include "Render_Struct.h"
#include "MeshRenderer.h"
#include "Component_Processor_Impl.h"

NS_BEGIN(Engine)

class CTransform_Processor;
class CAnimator_Processor;
class CRenderer_System;

class ENGINE_DLL CMeshRenderer_Processor final : public CComponent_Processor_Impl<CMeshRenderer, COMPONENT_TYPE::MESH_RENDERER>
{
    DEF_PROCESSOR_ID(PROCESSOR_ID::MESH_RENDERER)
public :
    CMeshRenderer_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    ~CMeshRenderer_Processor() override;
public:
    HRESULT Initialize() override;
    HRESULT Late_Initialize() override;
    void Update(_float fDT) override;
    void LateUpdate(_float fDT) override;
    void Build_RenderQueue(vector<DRAW_CMD>& outCmds);

    HRESULT Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec) override;
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;

    _bool   Resolve_References(COMPONENT_HANDLE hComponent);

    _bool   Resolve_AttachReference(COMPONENT_HANDLE hComponent);
    _bool   Resolve_SkinningReference(COMPONENT_HANDLE hComponent);
    void    Clear_AttachReference(MESH_RENDERER_DATA* pData);
    void    Clear_SkinningReference(MESH_RENDERER_DATA* pData);

private :
    void Initialize_Component_Data(COMPONENT_HANDLE hComponent) override;
    uint64_t Make_SortKey(const MESH_RENDERER_DATA& d) const;


    void    Reset_Data_On_Deallocate(COMPONENT_HANDLE hScript, MESH_RENDERER_DATA* pData);
    _bool   Build_Skinning_BoneRemap(MESH_RENDERER_DATA* pData);
    _bool   Build_SkinnedPart_BoneMatrices(MESH_RENDERER_DATA* pData);

    _bool   Find_Attach_BoneIndex(MESH_RENDERER_DATA* pData);
    _bool   Build_Attach_BoneMatrix(MESH_RENDERER_DATA* pData);

private :
    ID3D11Device*               m_pDevice{};
    ID3D11DeviceContext*        m_pContext{};
    CTransform_Processor*       m_pTransformProcessor{};
    CAnimator_Processor*        m_pAnimatorProcessor{};

public:
    static std::unique_ptr<CMeshRenderer_Processor> Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
};

NS_END
