#include "ConsolePanel.h"
#include "GUI_Sink.h"

#include "Editor_Util.h"
#include "Engine_Log.h"

NS_BEGIN(Editor)

CConsolePanel::CConsolePanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName), m_pSink(nullptr)
{
}

CConsolePanel::~CConsolePanel()
{
}

HRESULT CConsolePanel::Initialize()
{
    m_pSink = dynamic_cast<CGUI_Sink*>(SYS_LOG.Set_Sink(CLogger::LOG_TYPE::GUI));
    if (!m_pSink)
    {
        _DEBUG_ERROR_BREAK("CConsolePanel Init failed : m_pSink is nullptr");
        return E_FAIL;
    }
    return S_OK;
}

void CConsolePanel::Render()
{
    if (!m_bOpen)
        return;

    if (!ImGui::Begin(m_strPanelName.c_str(), (bool*)&m_bOpen))
    {
        ImGui::End();
        return;
    }

    Consume_From_Sink();
    Draw_Toolbar();

    ImGui::Separator();

    Draw_Log_List();

    ImGui::End();
}

void CConsolePanel::Consume_From_Sink()
{
    if (!m_pSink)
        return;

    m_tmpDrain.clear();
    m_pSink->Drain(m_tmpDrain);

    if (!m_tmpDrain.empty())
    {
        for (auto& r : m_tmpDrain)
            m_lines.push_back(std::move(r));

        _Enforce_Limit();

        if (m_bAutoScroll)
            m_bScrollToBottom = true;
    }
}

void CConsolePanel::_Enforce_Limit()
{
    while (m_lines.size() > m_iMaxLines)
        m_lines.pop_front();
}


void CConsolePanel::Draw_Toolbar()
{
    /* Clear */
    if (ImGui::Button("Clear"))
    {
        m_lines.clear();
        m_bScrollToBottom = true;
    }

    ImGui::SameLine();

    /* Copy to clipboard */
    if (ImGui::Button("Copy"))
    {
        ImGui::LogToClipboard();
        for (const auto& r : m_lines)
        {
            if (!Is_Visible_By_Filter(r))
                continue;

            ImGui::LogText("%s\n", r.strMsg.c_str());
        }
        ImGui::LogFinish();
    }

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_bAutoScroll);

    ImGui::SameLine();
    ImGui::TextUnformatted("Filter");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputText("##ConsoleFilter", &m_strFilter);
}

bool CConsolePanel::Is_Visible_By_Filter(const Engine::CLogger::RECORD& t) const
{
    if (m_strFilter.empty())
        return true;

    return Editor_Util::Str_IContains(t.strMsg, m_strFilter);
}

void CConsolePanel::Draw_Log_List()
{
    /* Extract only the indices of logs that pass the current filters */
    std::vector<size_t> filteredIndices;
    filteredIndices.reserve(m_lines.size());

    for (size_t i = 0; i < m_lines.size(); ++i)
    {
        if (Is_Visible_By_Filter(m_lines[i]))
        {
            filteredIndices.push_back(i);
        }
    }

    /* Scrollable area */
    ImGui::BeginChild("##ConsoleScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    /* The clipper must operate based on the filtered count, not the original m_lines */
    ImGuiListClipper clipper;
    clipper.Begin((int)filteredIndices.size());

    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& r = m_lines[filteredIndices[i]];

            _bool bHasColor = false;
            if (r.eSeverity == Engine::SEVERITY_TYPE::ERR)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); bHasColor = true;
            }
            else if (r.eSeverity == Engine::SEVERITY_TYPE::WARN)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.4f, 1.0f)); bHasColor = true;
            }

            ImGui::TextUnformatted(r.strMsg.c_str());

            if (bHasColor)
                ImGui::PopStyleColor();
        }
    }

    if (m_bScrollToBottom)
    {
        ImGui::SetScrollHereY(1.0f);
        m_bScrollToBottom = false;
    }

    ImGui::EndChild();
}

std::unique_ptr<CConsolePanel> CConsolePanel::Create(const std::string& strPanelName)
{
    auto pInstance = std::make_unique<CConsolePanel>(strPanelName);
    if (FAILED(pInstance->Initialize()))
    {
        _DEBUG_ERROR_BREAK("CConsolePanel Create failed");
        return nullptr;
    }
    return pInstance;
}

NS_END
