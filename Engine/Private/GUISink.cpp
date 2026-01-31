#include "GUISink.h"

NS_BEGIN(Engine)

CGUI_Sink::CGUI_Sink()
{
}

void CGUI_Sink::Write(const CLogger::RECORD& tRecord)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_queue.push_back(tRecord);
    Enforce_Limit_Locked();
}

void CGUI_Sink::Drain(std::vector<CLogger::RECORD>& out)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    if (m_queue.empty())
        return;

    out.reserve(out.size() + m_queue.size());
    while (!m_queue.empty())
    {
        out.push_back(std::move(m_queue.front()));
        m_queue.pop_front();
    }
}

void CGUI_Sink::Enforce_Limit_Locked()
{
    while (m_queue.size() > m_iMaxRecords)
        m_queue.pop_front();
}

NS_END
