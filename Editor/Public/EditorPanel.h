//#pragma once
//
//#include "Base.h"
//#include "GameInstance.h"
//#include "Editor_Define.h"
//
//namespace Engine
//{
//    class CGameInstance;
//}
//
//NS_BEGIN(Editor)
//
//class CEditorPanel : public CBase
//{
//public:
//    CEditorPanel(const std::string& title)
//        : _title(title), m_pGameInstance( CGameInstance::GetInstance() ), m_bOpen(true) { }
//    virtual ~CEditorPanel() = default;
//
//    virtual void Init() {}
//    virtual void Update() {}    
//    virtual void RenderUI() = 0;
//
//    const std::string& GetTitle() const { return _title; }
//    _bool IsOpen() const { return m_bOpen; }
//    void SetOpen(_bool open) { m_bOpen = open; }
//
//protected:
//    std::wstring OpenFileDialog(const wchar_t* filter);
//
//protected:
//    std::string _title;
//    Engine::CGameInstance*  m_pGameInstance {};
//    _bool m_bOpen;
//};
//
//NS_END
