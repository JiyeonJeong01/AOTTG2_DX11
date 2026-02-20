#pragma once
#include "EditorPanel.h"

struct ID3D11ShaderResourceView;

NS_BEGIN(Engine)
class CGameObject;
struct tagTransformData;
NS_END

NS_BEGIN(Editor)

class CGizmo;
class CHierarchyPanel;

class CScenePanel final : public CEditorPanel
{
public:
    CScenePanel(const std::string& strPanelName);
    ~CScenePanel() override;

public:
    HRESULT Initialize(CHierarchyPanel* pHierarchy);
    void    Update() override;
    void    Render() override;

private:
    void Draw_Toolbar();
    void Draw_Viewport();

    void Set_Target(Engine::CGameObject* pObject);

    void Ensure_RenderTarget(_uint w, _uint h);
    void Render_Scene(_uint w, _uint h);

private:
    // ImGui viewport state
    _uint   m_iViewW = 0;
    _uint   m_iViewH = 0;
    bool    m_bHovered = false;
    bool    m_bFocused = false;

    // show options
    bool    m_bShowGrid = true;
    bool    m_bShowAxis = true;

    Engine::CGameObject*        m_pTarget{};
    Engine::tagTransformData*   m_pData{};

private:
    // cached SRV ptr for ImGui::Image
    ID3D11ShaderResourceView* m_pSceneSRV = nullptr;
    std::unique_ptr<CGizmo> m_pGizmo;

public:
    static std::unique_ptr<CScenePanel> Create(const std::string& strPanelName, CHierarchyPanel* pHierarchy);
};

NS_END
