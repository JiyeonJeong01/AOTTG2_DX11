#pragma once
#include "EditorPanel.h"

NS_BEGIN(Editor)

class CResourcePanel final : public CEditorPanel
{
public:
    explicit CResourcePanel(const std::string& strPanelName);
    ~CResourcePanel();

public:
    HRESULT Initialize() override;
    void Update() override;
    void Render() override;

public:
    static std::unique_ptr<CResourcePanel> Create(const std::string& strPanelName);

private:
    void Draw_Search_Bar();
    void Draw_Result();

    Engine::ASSET_TYPE Get_Selected_AssetType() const;
    const char* Get_Selected_AssetType_Name() const;

private:
    _uint       m_iHandle = INVALID_HANDLE_UINT;
    int         m_iTypeIndex = 0;
    std::string m_searchName;

private:
    inline static constexpr const char* s_arrTypeNames[] =
    {
        "Mesh",
        "Model",
        "Material",
        "Shader",
        "Texture"
    };
};

NS_END
