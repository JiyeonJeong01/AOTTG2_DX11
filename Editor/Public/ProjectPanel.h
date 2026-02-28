#pragma once
#include "EditorPanel.h"
#include "Event.h"

NS_BEGIN(Editor)

class CProjectPanel final : public CEditorPanel
{
public:
    CProjectPanel(const std::string& strPanelName);
    ~CProjectPanel() override;

public:
    HRESULT Initialize() override;
    void    Update() override;
    void    Render() override;

public:
    /* TODO : ===================================================================================*/
    /* TODO : 이거 바꾸자!! Add 를 넣든 아니면 EventManager를 사용하든 public으로 두는 건 위험하다   */
    /* TODO : ===================================================================================*/
    CEvent<const ASSET_SELECTION&> m_OnSelectionChanged;

private:
    /* Selection */
    const std::filesystem::path&    Get_Selected_Path() const;
    _bool                           Has_Selection() const;
    void                            Set_Selection(const std::filesystem::path& path);
    void                            Clear_Selection();
    void                            Notify_Selection_Changed();
    ASSET_SELECTION                 Build_Selection(const std::filesystem::path& p) const;

    /* Rename */
    void    Begin_Rename(const std::filesystem::path& targetPath);
    void    Cancel_Rename();
    _bool   Is_Renaming() const;

    /* Main render section */
    void    Draw_Toolbar();
    void    Draw_Search_Bar();
    void    Draw_Main_Split();

    /* Left : folder tree */
    void    Draw_Folder_Tree();
    void    Draw_Folder_Node_Recursive(const FOLDER_NODE& tNode, _int iDepth);

    /* Right : file list */
    void    Draw_File_List();
    void    Draw_File_Asset_Row(const LIST_ASSET& tAsset);

    /* Context menu */
    void    Draw_Context_Menu();
    void    Draw_Context_Popup();
    _bool   Create_Folder(const std::filesystem::path& parentFolder, std::filesystem::path& outCreatedPath);
    void    Draw_Project_Context_Unified();

    /* Data refresh */
    void    Refresh_Folder_Tree();    /* Rebuild if dirty */
    void    Refresh_File_List();      /* Rebuild when folder changes or dirty */

    _bool   Is_Folder_Open(const std::filesystem::path& path) const;
    void    Set_Folder_Open(const std::filesystem::path& path, _bool bOpen);

    void    Ensure_Path_Exists(std::filesystem::path& inoutPath);

    /* Operations */
    _bool   Rename_Path(const std::filesystem::path& src, const std::string& newName);
    _bool   Delete_Path(const std::filesystem::path& target);
    void    Show_In_Explorer(const std::filesystem::path& target);
    std::filesystem::path    Resolve_Create_Base_Folder_() const;
    std::filesystem::path    Resolve_Create_Folder_By_Type_(const char* szType) const;


    /* Rename UI */
    void    Draw_Rename_Field(const LIST_ASSET& item);
    void    Commit_Rename();

    void  Try_Open_Scene_On_DoubleClick(const LIST_ASSET& tAsset);
    _bool Is_Scene_Asset(const LIST_ASSET& tAsset) const;
    _bool Try_Get_Asset_GUID(const std::filesystem::path& path, Engine::ASSET_GUID& outGuid) const;
    void  Open_Scene_By_GUID(const Engine::ASSET_GUID& guid, APP_MODE eMode);

    /* Scripts */
    void Draw_Create_Script_Popup();
    _bool Create_Script_By_Name(const std::string& baseStem, std::filesystem::path& outCreatedPath);
    std::string Make_Unique_File_Stem_Impl(const std::filesystem::path& parent, const std::string& baseStem);
    _bool Write_Text_File(const std::filesystem::path& p, const std::string& utf8);
    std::string Make_Script_File_From_Stem(const std::string& stem);

public :
    /* Helpers */
    static _bool        Is_Visible_By_Filter(const std::string& name, const std::string& filter);

private:
    /* Root */
    std::filesystem::path   m_assetsRoot;         /* absolute or normalized base */
    FOLDER_NODE             m_rootNode;

    /* Current folder */
    std::filesystem::path   m_currentFolder;

    /* Cached list */
    std::vector<LIST_ASSET> m_Assets;

    /* UI state */
    std::string             m_search;

    /* Tree open state */
    std::unordered_set<uint64_t> m_openFolders;

    /* Selection */
    std::filesystem::path   m_selectedPath;

    /* Context menu */
    std::filesystem::path   m_contextTargetPath;

    /* Create */
    std::filesystem::path   m_ScenePath;
    std::filesystem::path   m_PrototypePath;
    std::filesystem::path   m_MaterialPath;
    std::filesystem::path   m_ShaderPath;
    std::filesystem::path    m_ScriptPath;
    std::filesystem::path    m_HeaderPath;
    std::filesystem::path    m_ImplPath;
    _bool m_bOpenCreateScriptPopup = false;
    std::string m_createScriptNameBuffer = "New_Script";

    /* Rename */
    std::filesystem::path   m_renameTargetPath;
    std::string             m_renameBuffer;
    _bool                   m_bJustStartedRename = false;
    std::filesystem::path   m_pendingRenamePath{};

    /* Dirty flags */
    _bool m_bTreeDirty = true;
    _bool m_bListDirty = true;

    /* R-Button Popup */
    std::filesystem::path m_popupTargetPath;
    _bool m_popupTargetIsItem = false;

private:
    static uint64_t Get_Stable_Id_From_Path(const std::filesystem::path& p);
    static _bool    String_IContains(const std::string& haystack, const std::string& needle);
    static _bool    Is_Subpath(const std::filesystem::path& path, const std::filesystem::path& base);
    static std::string Make_Unique_Folder_Name_Impl(const std::filesystem::path& parent, const std::string& baseName);

public:
    static std::unique_ptr<CProjectPanel> Create(const std::string& strPanelName);
};


NS_END
