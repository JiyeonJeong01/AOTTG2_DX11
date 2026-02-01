#pragma once

#include "Base.h"
#include "GameInstance.h"
#include "Editor_Define.h"

namespace Engine
{
    class CCore_System;
}

NS_BEGIN(Editor)

class CEditorPanel : public CBase
{
public:
    CEditorPanel(const std::string& strPanel)
        : m_strPanelName(strPanel), m_bOpen(true) { }
    virtual ~CEditorPanel() = default;

    virtual HRESULT Initialize() { return S_OK; }
    virtual void Update() {}    
    virtual void Render() = 0;

    const std::string& GetTitle() const { return m_strPanelName; }
    _bool IsOpen() const { return m_bOpen; }
    void SetOpen(_bool open) { m_bOpen = open; }

protected:
    std::wstring OpenFileDialog(const wchar_t* filter) { return wstring{}; };

protected:
    std::string m_strPanelName;
    _bool m_bOpen;
};

NS_END
