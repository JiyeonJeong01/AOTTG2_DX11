//#include "HierarchyPanel.h"
//
//// 너 엔진/에디터 실제 include로 교체
//#include "GameObject.h"
//#include "Transform.h"
//#include "InspectorPanel.h"
//
//#include "EditorManager.h"
//#include "SceneView.h"
//#include "Utils.h"
//
//NS_BEGIN(Editor)
//
//CHierarchyPanel::CHierarchyPanel()
//    : CEditorPanel("Hierarchy")
//{
//    m_bOpen = true;
//    m_SearchBuf[0] = '\0';
//    m_CreateSearchBuf[0] = '\0';
//}
//
//void CHierarchyPanel::Bind_Inspector(Engine::CInspectorPanel* pInspector)
//{
//    m_pInspector = pInspector;
//}
//
//void CHierarchyPanel::Init()
//{
//    // GameObject 생성 이벤트 구독
//    EVENTS->Subscribe(EventType::GameObject_Created, [this](Event& e)
//        {
//            auto& ev = static_cast<GameObjectCreateEvent&>(e);
//            auto pObj = ev.GetGameObject(); // 너 이벤트가 CGameObject*를 준다고 가정(아니면 여기만 바꾸면 됨)
//            if (!pObj) return;
//
//            pObj = Ensure_Unique_Name(pObj);
//
//            CUR_SCENE->Add_Scene(pObj);
//            Add_Object(pObj);
//
//            LOG_WARNING("GameObject Created !");
//        });
//
//    // GameObject 삭제 이벤트 구독
//    EVENTS->Subscribe(EventType::GameObject_Destroyed, [this](Event& e)
//        {
//            auto& ev = static_cast<GameObjectDestroyedEvent&>(e);
//            auto pObj = ev.GetGameObject();
//            if (!pObj) return;
//
//            CUR_SCENE->Remove_Scene(pObj);
//            Remove_Object(pObj);
//
//            // 선택에서 제거
//            Remove_Selection(pObj);
//
//            // clipboard에서도 제거(선택 복붙 후 삭제 시 안전)
//            auto it = std::remove(m_Clipboard.begin(), m_Clipboard.end(), pObj);
//            if (it != m_Clipboard.end())
//                m_Clipboard.erase(it, m_Clipboard.end());
//
//            // Inspector sync
//            Sync_Inspector_Selection();
//
//            LOG_INFO("Destroyed GameObject");
//        });
//}
//
//void CHierarchyPanel::Update()
//{
//    // 필요하면 Scene의 오브젝트 목록을 매 프레임 동기화
//    // 지금은 이벤트 기반으로 Add/Remove 하니까 비워둠
//}
//
//void CHierarchyPanel::RenderUI()
//{
//    if (!m_bOpen) return;
//
//    if (!ImGui::Begin((_title + "##HierarchyPanel").c_str(), reinterpret_cast<bool*>(&m_bOpen),
//        ImGuiWindowFlags_NoNavInputs))
//    {
//        ImGui::End();
//        return;
//    }
//
//    // 입력은 패널이 포커스일 때만
//    if (ImGui::IsWindowFocused())
//    {
//        Handle_Input_Shortcuts();
//        Handle_Keyboard_Navigation();
//    }
//
//    Draw_Toolbar();
//    Draw_Object_List();
//
//    ImGui::End();
//}
//
//Engine::CGameObject* CHierarchyPanel::Get_Primary_Selection() const
//{
//    return m_Selected.empty() ? nullptr : m_Selected.front();
//}
//
//void CHierarchyPanel::Set_Selection_Single(Engine::CGameObject* pObj)
//{
//    m_Selected.clear();
//    if (pObj) m_Selected.push_back(pObj);
//    Sync_Inspector_Selection();
//}
//
//void CHierarchyPanel::Add_Selection(Engine::CGameObject* pObj)
//{
//    if (!pObj) return;
//    if (!Is_Selected(pObj))
//        m_Selected.push_back(pObj);
//}
//
//void CHierarchyPanel::Remove_Selection(Engine::CGameObject* pObj)
//{
//    if (!pObj) return;
//    auto it = std::find(m_Selected.begin(), m_Selected.end(), pObj);
//    if (it != m_Selected.end())
//        m_Selected.erase(it);
//}
//
//void CHierarchyPanel::Clear_Selection()
//{
//    m_Selected.clear();
//    Sync_Inspector_Selection();
//}
//
//_bool CHierarchyPanel::Is_Selected(Engine::CGameObject* pObj) const
//{
//    return std::find(m_Selected.begin(), m_Selected.end(), pObj) != m_Selected.end();
//}
//
//void CHierarchyPanel::Add_Object(Engine::CGameObject* pObj)
//{
//    if (!pObj) return;
//    m_SceneObjects.push_back(pObj);
//    m_bSortDirty = true;
//}
//
//void CHierarchyPanel::Remove_Object(Engine::CGameObject* pObj)
//{
//    if (!pObj) return;
//    auto it = std::find(m_SceneObjects.begin(), m_SceneObjects.end(), pObj);
//    if (it != m_SceneObjects.end())
//    {
//        m_SceneObjects.erase(it);
//        m_bSortDirty = true;
//    }
//}
//
//void CHierarchyPanel::Clear_Objects()
//{
//    m_SceneObjects.clear();
//    m_Selected.clear();
//    m_Clipboard.clear();
//    m_bSortDirty = true;
//    Sync_Inspector_Selection();
//}
//
//void CHierarchyPanel::Draw_Toolbar()
//{
//    if (ImGui::Button("Create Object"))
//        ImGui::OpenPopup("##CreateObjectPopup");
//
//    Draw_Create_Object_Popup();
//
//    ImGui::Separator();
//}
//
//void CHierarchyPanel::Draw_Create_Object_Popup()
//{
//    if (!ImGui::BeginPopup("##CreateObjectPopup"))
//        return;
//
//    ImGui::InputTextWithHint("##CreateSearch", "Search Class...", m_CreateSearchBuf, IM_ARRAYSIZE(m_CreateSearchBuf));
//    ImGui::Separator();
//
//    const std::string filter = m_CreateSearchBuf;
//
//    const auto& creators = GameObjectFactory::GetAllCreator();
//
//    for (const auto& [name, creator] : creators)
//    {
//        if (!filter.empty() && name.find(filter) == std::string::npos)
//            continue;
//
//        if (ImGui::Selectable(name.c_str()))
//        {
//            Engine::CGameObject* pNewObj = creator(); // creator가 raw pointer를 만든다고 가정
//            if (pNewObj)
//            {
//                if (pNewObj->GetName() == L"GameObject")
//                    pNewObj->SetName(Utils::ToWString(name));
//
//                pNewObj->GetOrAddTransform();
//
//                EVENTS->Publish(std::make_shared<GameObjectCreateEvent>(pNewObj));
//                Set_Selection_Single(pNewObj);
//            }
//
//            ImGui::CloseCurrentPopup();
//        }
//    }
//
//    ImGui::EndPopup();
//}
//
//void CHierarchyPanel::Draw_Object_List()
//{
//    Sort_If_Dirty();
//
//    ImGui::InputTextWithHint("##SearchHierarchy", "Search...", m_SearchBuf, IM_ARRAYSIZE(m_SearchBuf));
//    ImGui::Separator();
//
//    // 카운트
//    _int filtered = 0;
//    for (auto* pObj : m_SceneObjects)
//        if (Pass_Search_Filter(pObj)) filtered++;
//
//    ImGui::Text("Objects : %d / %d", filtered, (_int)m_SceneObjects.size());
//    ImGui::Separator();
//
//    for (_int i = 0; i < (_int)m_SceneObjects.size(); ++i)
//    {
//        Engine::CGameObject* pObj = m_SceneObjects[i];
//        if (!pObj) continue;
//        if (!Pass_Search_Filter(pObj)) continue;
//
//        const _bool isSelected = Is_Selected(pObj);
//
//        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
//        if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;
//
//        const std::string name = Utils::ToString(pObj->GetName());
//        ImGui::TreeNodeEx((void*)pObj, flags, "%s", name.c_str());
//
//        // 우클릭 메뉴
//        Draw_Object_Context_Menu(pObj);
//
//        // 클릭 선택
//        if (ImGui::IsItemClicked())
//        {
//            const _bool ctrl = INPUT->GetButton(KEY_TYPE::CTRL);
//            const _bool shift = INPUT->GetButton(KEY_TYPE::SHIFT);
//
//            if (ctrl)
//            {
//                // Ctrl + 클릭 : 토글
//                if (Is_Selected(pObj)) Remove_Selection(pObj);
//                else Add_Selection(pObj);
//            }
//            else if (shift)
//            {
//                // Shift + 클릭 : 범위 선택 (anchor = 첫 선택)
//                if (!m_Selected.empty())
//                {
//                    Engine::CGameObject* pAnchor = m_Selected.front();
//                    _int startIdx = 0;
//
//                    for (_int k = 0; k < (_int)m_SceneObjects.size(); ++k)
//                    {
//                        if (m_SceneObjects[k] == pAnchor) { startIdx = k; break; }
//                    }
//
//                    _int endIdx = i;
//                    _int a = std::min(startIdx, endIdx);
//                    _int b = std::max(startIdx, endIdx);
//
//                    m_Selected.clear();
//                    for (_int k = a; k <= b; ++k)
//                    {
//                        Engine::CGameObject* p = m_SceneObjects[k];
//                        if (p && Pass_Search_Filter(p))
//                            m_Selected.push_back(p);
//                    }
//                }
//                else
//                {
//                    m_Selected.clear();
//                    m_Selected.push_back(pObj);
//                }
//            }
//            else
//            {
//                // 일반 클릭 : 단일 선택
//                Set_Selection_Single(pObj);
//            }
//
//            Sync_Inspector_Selection();
//        }
//
//        // 더블 클릭 : 카메라 포커스
//        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
//        {
//            Focus_To_Object(pObj);
//        }
//    }
//}
//
//void CHierarchyPanel::Draw_Object_Context_Menu(Engine::CGameObject* pObj)
//{
//    if (!pObj) return;
//
//    if (ImGui::BeginPopupContextItem("##HierarchyItemContext"))
//    {
//        if (ImGui::Selectable("Duplicate"))
//        {
//            Engine::CGameObject* pClone = pObj->Clone();
//            if (pClone)
//            {
//                EVENTS->Publish(std::make_shared<GameObjectCreateEvent>(pClone));
//                Set_Selection_Single(pClone);
//            }
//        }
//
//        if (ImGui::Selectable("Delete"))
//        {
//            EVENTS->Publish(std::make_shared<GameObjectDestroyedEvent>(pObj));
//            // Destroy 이벤트에서 실제 리스트/선택 정리됨
//        }
//
//        ImGui::EndPopup();
//    }
//}
//
//void CHierarchyPanel::Handle_Input_Shortcuts()
//{
//    // Ctrl+C : 복사
//    if (INPUT->GetButton(KEY_TYPE::CTRL) && INPUT->GetButtonDown(KEY_TYPE::C))
//    {
//        m_Clipboard = m_Selected;
//        LOG_INFO("Copied " + std::to_string(m_Clipboard.size()) + " objects");
//    }
//
//    // Ctrl+V : 붙여넣기
//    if (INPUT->GetButton(KEY_TYPE::CTRL) && INPUT->GetButtonDown(KEY_TYPE::V))
//    {
//        std::vector<Engine::CGameObject*> clones;
//        for (auto* pOriginal : m_Clipboard)
//        {
//            if (!pOriginal) continue;
//            Engine::CGameObject* pClone = pOriginal->Clone();
//            if (pClone)
//            {
//                EVENTS->Publish(std::make_shared<GameObjectCreateEvent>(pClone));
//                clones.push_back(pClone);
//            }
//        }
//
//        m_Selected = clones;
//        Sync_Inspector_Selection();
//
//        LOG_INFO("Pasted " + std::to_string(clones.size()) + " objects");
//    }
//
//    // Ctrl+D : 바로 복제
//    if (INPUT->GetButton(KEY_TYPE::CTRL) && INPUT->GetButtonDown(KEY_TYPE::D))
//    {
//        std::vector<Engine::CGameObject*> clones;
//        for (auto* pOriginal : m_Selected)
//        {
//            if (!pOriginal) continue;
//            Engine::CGameObject* pClone = pOriginal->Clone();
//            if (pClone)
//            {
//                EVENTS->Publish(std::make_shared<GameObjectCreateEvent>(pClone));
//                clones.push_back(pClone);
//            }
//        }
//
//        m_Selected = clones;
//        Sync_Inspector_Selection();
//
//        LOG_INFO("Duplicated " + std::to_string(clones.size()) + " objects");
//    }
//
//    // Delete : 삭제
//    if (INPUT->GetButtonDown(KEY_TYPE::DEL))
//    {
//        auto toDelete = m_Selected; // 순회 중 수정 방지(복사)
//        for (auto* pObj : toDelete)
//        {
//            if (pObj)
//                EVENTS->Publish(std::make_shared<GameObjectDestroyedEvent>(pObj));
//        }
//        m_Selected.clear();
//        Sync_Inspector_Selection();
//
//        LOG_INFO("Deleted " + std::to_string(toDelete.size()) + " objects");
//    }
//
//    // Enter : 카메라 포커스 (선택 첫번째 기준)
//    if (INPUT->GetButtonDown(KEY_TYPE::ENTER))
//    {
//        auto* pObj = Get_Primary_Selection();
//        if (pObj) Focus_To_Object(pObj);
//    }
//}
//
//void CHierarchyPanel::Handle_Keyboard_Navigation()
//{
//    if (m_SceneObjects.empty())
//        return;
//
//    const _bool ctrl = INPUT->GetButton(KEY_TYPE::CTRL);
//
//    // 위
//    if (INPUT->GetButtonDown(KEY_TYPE::UP))
//    {
//        Move_Selection(-1, ctrl);
//        m_KeyRepeatTimer = 0.f;
//    }
//    else if (INPUT->GetButton(KEY_TYPE::UP))
//    {
//        m_KeyRepeatTimer += DT;
//        if (m_KeyRepeatTimer >= m_KeyRepeatDelay)
//        {
//            Move_Selection(-1, ctrl);
//            m_KeyRepeatTimer = 0.f;
//        }
//    }
//
//    // 아래
//    if (INPUT->GetButtonDown(KEY_TYPE::DOWN))
//    {
//        Move_Selection(1, ctrl);
//        m_KeyRepeatTimer = 0.f;
//    }
//    else if (INPUT->GetButton(KEY_TYPE::DOWN))
//    {
//        m_KeyRepeatTimer += DT;
//        if (m_KeyRepeatTimer >= m_KeyRepeatDelay)
//        {
//            Move_Selection(1, ctrl);
//            m_KeyRepeatTimer = 0.f;
//        }
//    }
//}
//
//_int CHierarchyPanel::Get_Current_Selected_Index() const
//{
//    if (m_Selected.empty() || m_SceneObjects.empty())
//        return -1;
//
//    Engine::CGameObject* pLast = m_Selected.back();
//    auto it = std::find(m_SceneObjects.begin(), m_SceneObjects.end(), pLast);
//    if (it == m_SceneObjects.end())
//        return -1;
//
//    return (_int)std::distance(m_SceneObjects.begin(), it);
//}
//
//void CHierarchyPanel::Move_Selection(_int iDirection, _bool bAdditive)
//{
//    if (m_SceneObjects.empty())
//        return;
//
//    _int cur = Get_Current_Selected_Index();
//    if (cur < 0) cur = 0;
//
//    _int next = std::max(0, std::min((_int)m_SceneObjects.size() - 1, cur + iDirection));
//    Engine::CGameObject* pObj = m_SceneObjects[next];
//    if (!pObj) return;
//
//    if (!bAdditive)
//    {
//        m_Selected.clear();
//        m_Selected.push_back(pObj);
//    }
//    else
//    {
//        if (!Is_Selected(pObj))
//            m_Selected.push_back(pObj);
//    }
//
//    Sync_Inspector_Selection();
//}
//
//void CHierarchyPanel::Focus_To_Object(Engine::CGameObject* pObj)
//{
//    if (!pObj) return;
//
//    auto sceneView = dynamic_pointer_cast<SceneView>(
//        GET_SINGLE(EditorManager)->GetWindow(L"Scene"));
//
//    if (sceneView && pObj->GetTransform())
//    {
//        Vec3 pos = pObj->GetTransform()->GetPosition();
//        sceneView->FocusOnPosition(pos);
//    }
//}
//
//_bool CHierarchyPanel::Pass_Search_Filter(Engine::CGameObject* pObj) const
//{
//    if (!pObj) return false;
//
//    const std::string filter = m_SearchBuf;
//    if (filter.empty())
//        return true;
//
//    const std::string name = Utils::ToString(pObj->GetName());
//    return name.find(filter) != std::string::npos;
//}
//
//Engine::CGameObject* CHierarchyPanel::Ensure_Unique_Name(Engine::CGameObject* pObj)
//{
//    if (!pObj) return nullptr;
//
//    auto& sceneObjects = CUR_SCENE->GetObjects(); // 여기가 vector<CGameObject*> 라고 가정
//
//    std::wstring baseName = pObj->GetName();
//    std::wstring uniqueName = baseName;
//    _int count = 1;
//
//    while (true)
//    {
//        _bool dup = false;
//
//        for (auto* p : sceneObjects)
//        {
//            if (!p || p == pObj) continue;
//            if (p->GetName() == uniqueName)
//            {
//                dup = true;
//                break;
//            }
//        }
//
//        if (!dup) break;
//
//        ++count;
//        uniqueName = baseName + L"_" + std::to_wstring(count);
//    }
//
//    pObj->SetName(uniqueName);
//    return pObj;
//}
//
//void CHierarchyPanel::Sort_If_Dirty()
//{
//    if (!m_bSortDirty) return;
//
//    std::sort(m_SceneObjects.begin(), m_SceneObjects.end(),
//        [](Engine::CGameObject* a, Engine::CGameObject* b)
//        {
//            if (!a || !b) return false;
//            return a->GetName() < b->GetName();
//        });
//
//    m_bSortDirty = false;
//}
//
//void CHierarchyPanel::Sync_Inspector_Selection()
//{
//    if (!m_pInspector) return;
//
//    if (m_Selected.empty())
//        m_pInspector->Set_Target(nullptr);
//    else
//        m_pInspector->Set_Target(m_Selected.front());
//}
//
//NS_END
