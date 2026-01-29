//// InspectorPanel.cpp
//#include "InspectorPanel.h"
//
//#include "imgui.h"
//
//#include "GameObject.h"
//#include "Transform.h"
//
//NS_BEGIN(Editor)
//
//static constexpr float kRadToDeg = 180.0f / 3.1415926535f;
//static constexpr float kDegToRad = 3.1415926535f / 180.0f;
//
//CInspectorPanel::CInspectorPanel()
//    : CEditorPanel("Inspector")
//{
//}
//
//void CInspectorPanel::Init()
//{
//    m_componentNames.clear();
//    //m_componentFactories.clear();
//
//    //m_componentNames.emplace_back("ModelRenderer");
//    ////m_componentFactories.emplace("ModelRenderer", []()
//    //    {
//    //        // 예시: 너가 쓰는 셰이더/경로 정책에 맞게 변경
//    //        auto shader = std::make_shared<Shader>(L"23. RenderDemo.fx");
//    //        return CComponent>(std::make_shared<ModelRenderer>(shader));
//    //    });
//
//    // 기본 버퍼 초기화
//    m_nameBuf[0] = '\0';
//}
//
//void CInspectorPanel::Update()
//{
//    // 필요하면 여기서 target 유효성/씬 변경 등 갱신
//}
//
//void CInspectorPanel::SetTarget(Engine::CGameObject* pTarget)
//{
//    m_pTarget = pTarget;
//    m_addSelected = -1;
//    m_openAddPopup = false;
//
//
//    //if (auto sp = m_pTarget.lock())
//    //{
//    //    CopyToFixedBuffer(m_nameBuf, sizeof(m_nameBuf), sp->GetName()); // GetName() 없으면 obj.m_name로
//    //}
//    //else
//    //{
//    //    m_nameBuf[0] = '\0';
//    //}
//
//    // pending remove flush
//    while (!m_pendingRemove.empty()) m_pendingRemove.pop();
//}
//
//void CInspectorPanel::RenderUI()
//{
//    if (!m_bOpen) return;
//
//    // 패널 타이틀은 부모가 들고 있으니 여기선 "##Inspector"로 고유 ID만 분리
//    if (!ImGui::Begin((_title + "##InspectorPanel").c_str(), reinterpret_cast<bool*>(&m_bOpen)))
//    {
//        ImGui::End();
//        return;
//    }
//
//    auto target = m_target.lock();
//    if (!target)
//    {
//        ImGui::TextDisabled("No Object Selected");
//        ImGui::End();
//        return;
//    }
//
//    DrawHeaderBar(*target);
//
//    ImGui::Separator();
//
//    DrawTransform(*target);
//
//    ImGui::Separator();
//
//    DrawComponents(*target);
//
//    ImGui::Separator();
//
//    DrawAddComponent(*target);
//
//    ImGui::End();
//}
//
//void CInspectorPanel::DrawHeaderBar(GameObject& obj)
//{
//    // Active 토글 + 이름 편집
//    bool active = obj.IsActive();
//    if (ImGui::Checkbox("##Active", &active))
//        obj.SetActive(active);
//
//    ImGui::SameLine();
//
//    ImGui::SetNextItemWidth(-1);
//    if (ImGui::InputText("##Name", m_nameBuf, sizeof(m_nameBuf), ImGuiInputTextFlags_EnterReturnsTrue))
//    {
//        obj.SetName(std::string(m_nameBuf));
//    }
//
//    // 엔터 외에도 “포커스 잃었을 때” 커밋하고 싶으면 아래 추가
//    if (ImGui::IsItemDeactivatedAfterEdit())
//        obj.SetName(std::string(m_nameBuf));
//}
//
//void CInspectorPanel::DrawTransform(GameObject& obj)
//{
//    auto tr = obj.GetTransform();
//    if (!tr) return;
//
//    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
//        return;
//
//    Vec3 pos = tr->GetPosition();
//    Vec3 rot = tr->GetRotation();   // radians
//    Vec3 scl = tr->GetScale();
//
//    if (ImGui::DragFloat3("Position", reinterpret_cast<float*>(&pos), 0.1f))
//        tr->SetPosition(pos);
//
//    Vec3 rotDeg = rot * kRadToDeg;
//    if (ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&rotDeg), 1.0f))
//        tr->SetRotation(rotDeg * kDegToRad);
//
//    if (ImGui::DragFloat3("Scale", reinterpret_cast<float*>(&scl), 0.01f))
//        tr->SetScale(scl);
//}
//
//void CInspectorPanel::DrawComponents(GameObject& obj)
//{
//    // GameObject가 컴포넌트 목록을 제공한다고 가정
//    // - 예: obj.GetComponents() -> vector<shared_ptr<Component>>&
//    // - 삭제는 index 기반/ptr 기반 등 너 구조에 맞게 바꿔
//    auto& comps = obj.GetComponents();
//
//    for (size_t i = 0; i < comps.size(); ++i)
//    {
//        auto& c = comps[i];
//        if (!c) continue;
//
//        // 이름은 RTTI/TypeInfo/고정 문자열 등 너 방식에 맞추면 됨
//        const char* compName = c->GetTypeName(); // 없으면 c->GetTypeInfo().GetName() 같은 걸로
//
//        // “컴포넌트별 UI”는 컴포넌트가 직접 그리게 하면 깔끔함
//        // ex) virtual bool DrawInspector() / OnInspectorGUI()
//        ImGui::PushID(static_cast<int>(i));
//        bool open = ImGui::CollapsingHeader(compName, ImGuiTreeNodeFlags_DefaultOpen);
//
//        if (ImGui::BeginPopupContextItem("ComponentContext"))
//        {
//            if (ImGui::Selectable("Remove"))
//            {
//                m_pendingRemove.push(i);
//            }
//            ImGui::EndPopup();
//        }
//
//        if (open)
//        {
//            // 컴포넌트가 자신 UI를 렌더
//            // 반환값으로 값 변경 여부 받고, 변경 시 반영(Commit) 가능
//            bool changed = c->OnInspectorGUI();
//            if (changed)
//                c->ApplyEditorChanges(); // 없으면 제거해도 됨
//        }
//
//        ImGui::PopID();
//    }
//
//    // 안전하게 삭제 처리(역순 삭제)
//    if (!m_pendingRemove.empty())
//    {
//        std::vector<size_t> removeIdx;
//        while (!m_pendingRemove.empty())
//        {
//            removeIdx.push_back(m_pendingRemove.front());
//            m_pendingRemove.pop();
//        }
//        std::sort(removeIdx.begin(), removeIdx.end(), std::greater<size_t>());
//
//        for (size_t idx : removeIdx)
//        {
//            if (idx < comps.size())
//                obj.RemoveComponentAt(idx); // 너 구조에 맞게 구현/교체
//        }
//    }
//}
//
//void CInspectorPanel::DrawAddComponent(GameObject& obj)
//{
//    // 버튼 + 팝업 + 리스트 (두 코드의 장점을 합친 형태)
//    if (ImGui::Button("Add Component", ImVec2(-1, 30)))
//        m_openAddPopup = true;
//
//    if (m_openAddPopup)
//    {
//        ImGui::OpenPopup("##AddComponentPopup");
//        m_openAddPopup = false;
//    }
//
//    if (ImGui::BeginPopup("##AddComponentPopup"))
//    {
//        // 검색 필터 넣고 싶으면 여기서 InputText + 필터링하면 됨
//        ImGui::TextDisabled("Select a component to add");
//        ImGui::Separator();
//
//        // ListBox 형태 (복수 등록 대비 쉬움)
//        if (!m_componentNames.empty())
//        {
//            // ImGui::ListBox는 const char* 배열을 선호 -> 임시 벡터 구성
//            std::vector<const char*> items;
//            items.reserve(m_componentNames.size());
//            for (auto& s : m_componentNames) items.push_back(s.c_str());
//
//            ImGui::SetNextItemWidth(-1);
//            if (ImGui::ListBox("##ComponentList", &m_addSelected, items.data(), (int)items.size(), 10))
//            {
//                if (m_addSelected >= 0 && m_addSelected < (int)m_componentNames.size())
//                {
//                    const std::string& name = m_componentNames[m_addSelected];
//                    auto it = m_componentFactories.find(name);
//                    if (it != m_componentFactories.end())
//                    {
//                        auto comp = it->second();
//                        if (comp)
//                            obj.AddComponent(comp);
//                    }
//                    ImGui::CloseCurrentPopup();
//                }
//            }
//        }
//        else
//        {
//            ImGui::TextDisabled("No component registered.");
//        }
//
//        ImGui::EndPopup();
//    }
//}
//
//void CInspectorPanel::CopyToFixedBuffer(char* dst, size_t dstSize, const std::string& src)
//{
//    if (!dst || dstSize == 0) return;
//    const size_t n = std::min(dstSize - 1, src.size());
//    memcpy(dst, src.data(), n);
//    dst[n] = '\0';
//}
//
//NS_END // namespace Editor
