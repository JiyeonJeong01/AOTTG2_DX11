#pragma once
#include "Base.h"
#include "ISink.h"   // 여기에 ISink, RECORD 정의돼있다고 가정

#include <deque>
#include <mutex>
#include <vector>

NS_BEGIN(Engine)

class ENGINE_DLL CGUI_Sink final : public ISink
{
public:
    CGUI_Sink();
    ~CGUI_Sink() override = default;

    void Write(const CLogger::RECORD& tRecord) override;

    void Drain(std::vector<CLogger::RECORD>& out);

    void Set_Max_Records(size_t iMax)
    {
        m_iMaxRecords = (iMax == 0 ? 1 : iMax); Enforce_Limit_Locked();
    }
    size_t Get_Max_Records() const { return m_iMaxRecords; }

private:
    void Enforce_Limit_Locked();

private:
    std::mutex m_mtx;
    std::deque<CLogger::RECORD> m_queue;  // 아직 UI로 전달 안 된 것
    size_t m_iMaxRecords = 5000;
};

NS_END
