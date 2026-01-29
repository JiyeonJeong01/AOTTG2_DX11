//#pragma once
//#include "EditorPanel.h"
//
//
//NS_BEGIN(Engine)
//class CGameObject;
//NS_END
//
//NS_BEGIN(Editor)
//
//class CInspectorPanel :public CEditorPanel
//{
//public:
//    CInspectorPanel();
//    ~CInspectorPanel() override = default;
//
//    void Init() override;
//    void Update() override;
//    void RenderUI() override;
//
//    // 외부(Hierarchy 등)에서 선택 대상 주입
//    void SetTarget(Engine::CGameObject* pTarget);
//    Engine::CGameObject* GetTarget() const { return m_pTarget; }
//
//private:
//    void DrawHeaderBar(Engine::CGameObject& obj);
//    void DrawTransform(Engine::CGameObject& obj);
//    void DrawComponents(Engine::CGameObject& obj);
//    void DrawAddComponent(Engine::CGameObject& obj);
//
//    // UI helpers
//    static void CopyToFixedBuffer(char* dst, size_t dstSize, const std::string& src);
//
//private:
//    Engine::CGameObject* m_pTarget{};
//    _char m_nameBuf[128]{};
//    _int  m_addSelected = -1;
//    _bool m_openAddPopup = false;
//
//    // 삭제 요청 큐(즉시 erase하면 iter invalid/런타임 위험)
//    std::queue<size_t> m_pendingRemove;
//
//    std::vector<std::string> m_componentNames;
//    // std::unordered_map<std::string, ComponentFactoryFn> m_componentFactories;
//};
//
//NS_END
