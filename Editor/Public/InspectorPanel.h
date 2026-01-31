#pragma once
#include "EditorPanel.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Editor)

class CInspectorPanel final : public CEditorPanel
{
public:
    CInspectorPanel();
    ~CInspectorPanel() override = default;

    void Init(class CHierarchyPanel* pPanel);
    void Update() override {}
    void Render_UI() override;

    /* Binding with HierarchyPanel */
    void Set_Target(Engine::CGameObject* pObj);
    Engine::CGameObject* Get_Target() const
    {
        return m_pTarget;
    }

private:
    void Draw_Header();
    void Draw_Basic_Info();
    void Draw_Transform();
    void Draw_Components();     /* TODO : engine-specific */

    void Validate_Target();

private:
    Engine::CGameObject* m_pTarget = nullptr;

    /* UI */
    std::string m_nameBuffer;
    bool m_bJustStartedNameEdit = false;
    bool m_bAutoFocusOnSelection = true;

    // window
    const char* m_pszWindowName = "Inspector";
};

NS_END
