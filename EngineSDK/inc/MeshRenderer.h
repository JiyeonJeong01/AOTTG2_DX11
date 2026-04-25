#pragma once
#include "CComponent_Proxy_Base.h"
#include "Engine_Math.h"
#include "Render_Struct.h"

NS_BEGIN(Engine)

typedef struct tagParticleRuntime
{
    Microsoft::WRL::ComPtr<ID3D11Buffer> pPointVB;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pInstanceVB;

    uint32_t        hTexture = INVALID_HANDLE_UINT;

    _uint           iInstanceStride = sizeof(VTXPARTICLE_INSTANCE);
    _uint           iNumInstances = 0;

    std::vector<VTXPARTICLE_INSTANCE> vecInstances;
    std::vector<_float> vecSpeeds;

    _bool           bInitialized = false;
    std::vector<_float4> vecDirections;
} PARTICLE_RUNTIME;

typedef struct ENGINE_DLL tagMeshRendererData final
{
    OBJECT_HANDLE       hObject{};
    _bool               bEnable = false;
    SHADOW_TYPE         eShadowType = SHADOW_TYPE::STATIC;
    COMPONENT_HANDLE    hTransform = INVALID_HANDLE;
    COMPONENT_HANDLE    hAnimator = INVALID_HANDLE;
    COMPONENT_HANDLE    hSkinningSourceAnimator = INVALID_HANDLE;
    COMPONENT_HANDLE    hAttachSourceAnimator = INVALID_HANDLE;

    uint32_t            hMesh = INVALID_HANDLE_UINT;
    uint32_t            hMaterial = INVALID_HANDLE_UINT;
    uint32_t            hPerObjectParams = INVALID_HANDLE_UINT;
    uint32_t            hParticle = INVALID_HANDLE_UINT;

    uint32_t            iParticleRuntime = INVALID_HANDLE_UINT;

    uint32_t            flags = RF_NONE;
    RENDER_LAYER        layer = RENDER_LAYER::NONBLEND;

    _float              sortZ = 0.f;

    MESH_MODE               eMode = MESH_MODE::NONE;    /* NONE / PARTS / ATTACH */

    std::vector<uint32_t>   vecSkinningBoneRemap;      /* 파츠 bone index -> 부모 bone index */
    std::vector<_float4x4>  vecSkinningBoneMatrices;   /* 파츠용으로 재조립한 최종 bone matrices */

    uint32_t                iAttachBoneIdx = INVALID_HANDLE_UINT;
    std::string             strAttachBoneName;
    _float4x4               matFinalAttach = Math::Identity();

    /* -------- EXTRA PASS -------- */
    uint32_t            extraPassFlags = 0;

    /* -------- 파트별 머터리얼 오버라이드 -------- */
    std::vector<uint32_t>   vecOverrideMaterials;

    /* -------- PARTICLE -------- */
    _bool                   bParticlePlaying = false;
    _bool                   bParticleResetRequested = false;
    _bool                   bParticleFinished = false;
    _float3                 vParticlePivot{};
    _float3                 vParticleForward = { 0.f, 0.f, 1.f };

} MESH_RENDERER_DATA;

class ENGINE_DLL CMeshRenderer : public CComponent_Proxy_Base<MESH_RENDERER_DATA, CMeshRenderer, COMPONENT_TYPE::MESH_RENDERER>
{
public :
    CMeshRenderer() : CComponent_Proxy_Base() { }
    CMeshRenderer(DataType* pData, COMPONENT_HANDLE handle)
        : CComponent_Proxy_Base(pData, handle) {  }
    ~CMeshRenderer() override = default;

public:
    // Settings
    void Set_Mesh(uint32_t h);
    void Set_Material(uint32_t h);

    void Set_Layer(RENDER_LAYER e);
    void Set_Flags(uint32_t f);
    void Add_Flags(uint32_t f);
    void Remove_Flags(uint32_t f);
    void Set_SortZ(float z);


    // Getters
    COMPONENT_HANDLE Get_Transform() const;
    uint32_t         Get_Mesh() const;
    uint32_t         Get_Material() const;
    uint32_t         Get_Flags() const;
    RENDER_LAYER     Get_Layer() const;
    float            Get_SortZ() const;

public:
    void Set_Mode(MESH_MODE e);
    void Set_Particle(uint32_t hParticle);
    void Set_ParticlePlaying(_bool bPlaying);
    void Set_ParticlePivot(const _float3& vPivot);
    void Reset_Particle();
    void Stop_Particle();
    _bool Is_ParticleFinished() const;

    MESH_MODE Get_Mode() const;
    uint32_t Get_Particle() const;
    _bool Get_ParticlePlaying() const;
    const _float3& Get_ParticlePivot() const;
    void Set_ParticleForward(const _float3& vForward);
    const _float3& Get_ParticleForward() const;
};

NS_END
