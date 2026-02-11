#pragma once
#include "EditorPanel.h"

struct ID3D11ShaderResourceView;

NS_BEGIN(Editor)

class CScenePanel final : public CEditorPanel
{
public:
    CScenePanel(const std::string& strPanelName);
    ~CScenePanel() override;

public:
    HRESULT Initialize() override;
    void    Update() override;
    void    Render() override;

private:
    void Draw_Toolbar();
    void Draw_Viewport();

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

private:
    // cached SRV ptr for ImGui::Image
    ID3D11ShaderResourceView* m_pSceneSRV = nullptr;

public:
    static std::unique_ptr<CScenePanel> Create(const std::string& strPanelName);
};

NS_END
