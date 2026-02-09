#pragma once
#include "EditorPanel.h"
#include "Logger.h"   
namespace Engine
{
    class CGUI_Sink;
    class CLogger;
}

NS_BEGIN(Editor)
class CConsolePanel : public CEditorPanel
{
public :
    CConsolePanel(const std::string& strPanelName);
    ~CConsolePanel() override;

public :
    HRESULT Initialize() override;
    void Update() override {}
    void Render() override;

    void Set_Max_Lines(size_t iMax)
    {
        m_iMaxLines = (iMax == 0 ? 1 : iMax);
        _Enforce_Limit();
    }
    size_t Get_Max_Lines() const
    {
        return m_iMaxLines;
    }

private:
    void Draw_Toolbar();
    void Draw_Log_List();
    void Consume_From_Sink();

    bool Is_Visible_By_Filter(const Engine::CLogger::RECORD& t) const;
    void _Enforce_Limit();

private:
    CGUI_Sink* m_pSink = nullptr;

    std::deque<Engine::CLogger::RECORD> m_lines;
    size_t m_iMaxLines = 2000;

    std::string m_strFilter;
    bool m_bAutoScroll = true;
    bool m_bScrollToBottom = false;

    std::vector<Engine::CLogger::RECORD> m_tmpDrain;

public :
    static std::unique_ptr<CConsolePanel> Create(const std::string& strPanelName);
};

NS_END
