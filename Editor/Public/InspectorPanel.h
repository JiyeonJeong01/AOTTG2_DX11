#pragma once
#include "EditorPanel.h"
#include "ProjectPanel.h"

NS_BEGIN(Engine)
    class CGameObject;
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
    void Draw_Asset();
    void Draw_None();

    /* Draw components by type  */
    void Draw_ComponentByType(COMPONENT_TYPE eComType);

    void Draw_Transform();
    void Draw_RectTransform();
    void Draw_MeshRenderer();
    void Draw_CanvasRenderer();



    void Validate_Target();

private:
    Engine::CGameObject* m_pTarget = nullptr;

    /* UI */
    std::string m_nameBuffer;
    bool m_bJustStartedNameEdit = false;
    bool m_bAutoFocusOnSelection = true;

    // 신규 타겟
    ASSET_SELECTION m_selectedAsset;

public:
    static std::unique_ptr<CInspectorPanel> Create(const std::string& strPanelName, CHierarchyPanel* pHierarchy, CProjectPanel* pProject);
};

NS_END
