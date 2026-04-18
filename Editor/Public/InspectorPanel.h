#pragma once
#include <MeshRenderer.h>

#include "EditorPanel.h"
#include "ProjectPanel.h"

NS_BEGIN(Engine)
class CGameObject;
class CScript_Processor;
class IScript;
class CMeshRenderer_Processor;
NS_END

NS_BEGIN(Editor)

class CInspectorPanel final : public CEditorPanel
{
public:
    CInspectorPanel(const std::string& strPanelName);
    ~CInspectorPanel() override;

public :
    HRESULT Initialize(class CHierarchyPanel* pPanel, CProjectPanel* pProject);
    void Update() override {}
    void Render() override;

    /* Binding with HierarchyPanel */
    void Set_Target(Engine::CGameObject* pObj);
    Engine::CGameObject* Get_Target() const
    {
        return m_pTarget;
    }
    // 신규
    void Set_Selected_Asset(const ASSET_SELECTION& sel);
    void Clear_Target();

private:
    enum class InspectMode : uint8_t
    {
        None = 0,
        GameObject,
        Asset,
    };
    InspectMode m_eMode = InspectMode::None;

private:
    void Draw_Header();
    void Draw_Basic_Info();
    void Draw_ObjectLayer();
    void Draw_LayerEditorPopup();
    void Draw_RequiredComponent();
    void Draw_Components();
    void Draw_CurrentComponents();
    void Draw_AddComponentPopup();
    void Draw_CreateScriptPopup();
    void Draw_Asset();
    void Draw_None();

    /* Draw components by type  */
    void Draw_ComponentByType(COMPONENT_TYPE eComType);

    void Draw_Transform();
    void Draw_RectTransform();
    void Draw_MeshRenderer();
    void Draw_CanvasRenderer();
    void Draw_Collider();
    void Draw_ColliderMaskEditor(const char* szLabel, uint32_t& iTargetMask, bool& bChanged);

    void Draw_Rigidbody();
    void Draw_SpringJoint();
    void Draw_Animator();
    void Draw_Camera();
    void Draw_Script();
    void Draw_AllScripts(COMPONENT_HANDLE hComponent);
    void Draw_ScriptFields(class Engine::IScript* pScript);
    void Draw_UIImage();
    void Draw_UIButton();
    void Draw_UIText();
    void Draw_SpriteEffect();

    void Draw_AnimatorBlendingView();
    void Draw_AnimatorLoopView();

    void Validate_Target();

private:
    Engine::CScript_Processor* m_pScript_Processor = nullptr;
    Engine::CGameObject* m_pTarget = nullptr;

    /* UI */
    std::string m_nameBuffer;
    bool m_bJustStartedNameEdit = false;
    bool m_bAutoFocusOnSelection = true;

    // 신규 타겟
    ASSET_SELECTION m_selectedAsset;

    Engine::COMPONENT_HANDLE m_hPendingScript{};
    _bool m_bOpenCreateScriptPopup = false;
    std::string m_newScriptName;
    
    /* Mesh Renderer */
    Engine::CMeshRenderer_Processor*    m_pMeshRenderer_Processor = nullptr;
    _bool                               m_bAttachModeInitialized = false;

    bool            m_bAttachInputActive = false;
    bool            m_bEditUseAttach = false;
    MESH_MODE       m_eEditMode = MESH_MODE::NONE;
    _bool           m_bEditModeInitialized = false;
    std::string     m_strEditAttachBoneName = "";
    uint32_t        m_iEditParticleHandle = INVALID_HANDLE_UINT;

    /* Transform */
    _float3      m_vCachedRotationEuler{};
    CGameObject* m_pCachedTransformTarget = nullptr;

    static constexpr const char* m_layerNames[5] = {
        "PRIORITY",
        "NONBLEND",
        "BLEND",
        "UI",
        "SKY"
    };

    struct MASK_ITEM
    {
        const char* szName = "";
        uint32_t    iMask = 0;
    };

    static const std::array<MASK_ITEM, 12> s_arrColliderMaskItems;

public:
    static std::unique_ptr<CInspectorPanel> Create(const std::string& strPanelName, CHierarchyPanel* pHierarchy, CProjectPanel* pProject);
};

NS_END
