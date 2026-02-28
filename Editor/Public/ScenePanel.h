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

    void Ensure_RenderTarget();
    void Render_Scene(_uint w, _uint h);

private:
    /* NOTE : 현재 엔진에서는 해상도 무조건 1920 * 1080 고정 */
    const _uint   m_FIXEDW = 1920;
    const _uint   m_FIXEDH = 1080;
    _bool    m_bHovered = false;
    _bool    m_bFocused = false;

    // show options
    _bool    m_bShowGrid = true;
    _bool    m_bShowAxis = true;

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
