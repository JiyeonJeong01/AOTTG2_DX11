#include "InspectorPanel.h"
#include "HierarchyPanel.h"
#include "GameObject.h"
#include "Engine_Log.h"
#include "Asset_Registry.h"
#include "LayerHelper.h"

#include "Transform.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "RectTransform.h"
#include "CanvasRenderer.h"
#include "Script.h"
#include "Component_System.h"
#include "Editor_Util.h"
#include "Script_Processor.h"


NS_BEGIN(Editor)

CInspectorPanel::CInspectorPanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName)
{
}

CInspectorPanel::~CInspectorPanel()
{
}

HRESULT CInspectorPanel::Initialize(CHierarchyPanel* pPanel, CProjectPanel* pProject)
{
    pPanel->m_OnPrimarySelectionChanged.Add_Listener(&CInspectorPanel::Set_Target, this);
    pProject->m_OnSelectionChanged.Add_Listener(&CInspectorPanel::Set_Selected_Asset, this);

    CComponent_Processor* pBase = nullptr;
    SYS_COMPONENT.Bind_ComponentProcessor(COMPONENT_TYPE::SCRIPT, &pBase);
    IF_NULL_RETURN_MSG_BREAK(pBase, E_FAIL, "Transform processor bind failed");
    m_pScript_Processor = SCAST(CScript_Processor*, pBase);
    return S_OK;
}

void CInspectorPanel::Set_Target(Engine::CGameObject* pObj)
{
    if (m_pTarget == pObj)
        return;

    m_pTarget = pObj;

    /* Refresh name buffer */
    m_nameBuffer.clear();
    if (m_pTarget)
        m_nameBuffer.assign(m_pTarget->Get_Label());

    _DEBUG_INFO("Changed target");

    m_bJustStartedNameEdit = false;

    m_selectedAsset = ASSET_SELECTION{};
    m_eMode = (pObj ? InspectMode::GameObject : InspectMode::None);
}

void CInspectorPanel::Set_Selected_Asset(const ASSET_SELECTION& sel)
{
    m_selectedAsset = sel;
    m_pTarget = nullptr;
    m_eMode = (sel.Is_Valid() ? InspectMode::Asset : InspectMode::None);
}

void CInspectorPanel::Clear_Target()
{
}

void CInspectorPanel::Validate_Target()
{
    if (m_pTarget == nullptr)
        return;

    /* TODO : Validiate  */
}

void CInspectorPanel::Render()
{
    if (!m_bOpen)
        return;

    Validate_Target();

    if (!ImGui::Begin(m_strPanelName.c_str(), (bool*)&m_bOpen))
    {
        ImGui::End();
        return;
    }

    if (m_eMode == InspectMode::GameObject)
    {
        if (m_pTarget == nullptr)
        {
            ImGui::TextUnformatted("No selection.");
            ImGui::End();
            return;
        }
        Draw_Header();
        ImGui::Separator();

        Draw_ObjectLayer();
        ImGui::Separator();

        Draw_Basic_Info();
        ImGui::Separator();

        Draw_RequiredComponent();
        ImGui::Separator();

        Draw_Components();
    }
    else if (m_eMode == InspectMode::Asset)
    {
        Draw_Asset();
    }
    else
    {
        Draw_None();
    }

    ImGui::End();
}

void CInspectorPanel::Draw_Header()
{
    /* Optional : icon or breadcrumbs */
    ImGui::TextUnformatted(m_pTarget->Get_Label().data());
    ImGui::SameLine();
    ImGui::TextDisabled("(%p)", (void*)m_pTarget);

    ImGui::SameLine();
    ImGui::Checkbox("Auto Focus", &m_bAutoFocusOnSelection);
}

void CInspectorPanel::Draw_Basic_Info()
{
    /* Active Toggle */
    _bool bActive = m_pTarget->Get_Enabled();
    if (ImGui::Checkbox("Active", &bActive))
        m_pTarget->Set_Enable(bActive);

    /* Name Edit */
    ImGui::TextUnformatted("Name");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);

    if (m_bAutoFocusOnSelection && m_bJustStartedNameEdit)
    {
        ImGui::SetKeyboardFocusHere();
    }

    ImGuiInputTextFlags flags =
        ImGuiInputTextFlags_AutoSelectAll |
        ImGuiInputTextFlags_EnterReturnsTrue;

    const bool bEnter = ImGui::InputText("##InspectorName", &m_nameBuffer, flags);

    /* Confirm the name when enter is pressed */
    if (bEnter)
    {
        if (m_nameBuffer.empty())
            m_nameBuffer = "GameObject";

        m_pTarget->Set_Label(m_nameBuffer.c_str());
        m_bJustStartedNameEdit = false;
        return;
    }

    /* Skip the first frame to avoid race */
    if (m_bJustStartedNameEdit)
    {
        m_bJustStartedNameEdit = false;
        return;
    }

    /* Conform the name when focus is lost */
    if (ImGui::IsItemDeactivated())
    {
        if (m_nameBuffer.empty())
            m_nameBuffer = "GameObject";

        m_pTarget->Set_Label(m_nameBuffer.c_str());
        return;
    }
}

void CInspectorPanel::Draw_ObjectLayer()
{
    if (!m_pTarget) return;

    auto& sys = SYS_GAMEOBJECT;
    auto& layers = sys.Layers();

    const Layer::LAYER_ID curLayer = m_pTarget->Get_Layer();

    std::string curName = "None";
    layers.Find_Name(curLayer, curName);

    ImGui::TextUnformatted("Layer");
    ImGui::SameLine();

    ImGui::SetNextItemWidth(180.f);

    bool openEditPopup = false;

    if (ImGui::BeginCombo("##LayerCombo", curName.c_str()))
    {
        for (Layer::LAYER_ID i = 0; i < Layer::MAX_LAYERS; ++i)
        {
            if (!layers.Is_Used(i)) continue;

            std::string name;
            if (!layers.Find_Name(i, name)) continue;

            const bool selected = (i == curLayer);
            if (ImGui::Selectable(name.c_str(), selected))
                sys.Set_Layer(m_pTarget, i);

            if (selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::Separator();

        if (ImGui::Selectable("Edit Layers..."))
            openEditPopup = true;

        ImGui::EndCombo();
    }

    if (openEditPopup)
    {
        ImVec2 itemMin = ImGui::GetItemRectMin();
        ImVec2 itemSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos(
            ImVec2(itemMin.x, itemMin.y + itemSize.y),
            ImGuiCond_Appearing
        );

        ImGui::SetNextWindowSize(ImVec2(460.f, 420.f), ImGuiCond_Appearing);

        ImGui::OpenPopup("LayerEditorPopup");
    }

    Draw_LayerEditorPopup();
}

void CInspectorPanel::Draw_LayerEditorPopup()
{
    if (!ImGui::BeginPopup("LayerEditorPopup", ImGuiWindowFlags_AlwaysAutoResize))
        return;

    auto& layers = SYS_GAMEOBJECT.Layers();

    ImGui::TextUnformatted("Layers");
    ImGui::SameLine();

    float buttonWidth = 60.f;
    float padding = ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth - padding - 10.f);

    if (ImGui::Button("X", ImVec2(buttonWidth, 0)))
        ImGui::CloseCurrentPopup();

    ImGui::Separator();

    const float listH = 260.f;

    if (ImGui::BeginChild("##LayerList", ImVec2(420.f, listH), true))
    {
        for (Layer::LAYER_ID i = 0; i < Layer::MAX_LAYERS; ++i)
        {
            if (!layers.Is_Used(i))
                continue;

            std::string name;
            layers.Find_Name(i, name);

            const bool reserved = (i == Layer::DEFAULT_LAYER || i == Layer::UI_LAYER);

            ImGui::PushID((int)i);

            ImGui::Text("[%02u]", (unsigned)i);
            ImGui::SameLine();

            if (reserved)
            {
                ImGui::BeginDisabled(true);
                char buf[64]{};
                strncpy_s(buf, name.c_str(), _TRUNCATE);
                ImGui::SetNextItemWidth(260.f);
                ImGui::InputText("##LayerName", buf, IM_ARRAYSIZE(buf));
                ImGui::EndDisabled();

                ImGui::SameLine();
                ImGui::TextDisabled("System");
            }
            else
            {
                static char editBuf[Layer::MAX_LAYERS][64] = {};
                if (editBuf[i][0] == '\0')
                    strncpy_s(editBuf[i], name.c_str(), _TRUNCATE);

                ImGui::SetNextItemWidth(260.f);
                if (ImGui::InputText("##LayerName", editBuf[i], IM_ARRAYSIZE(editBuf[i]),
                    ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    std::string newName = editBuf[i];
                    if (!newName.empty() && newName != name)
                        layers.Rename_Layer(i, newName);
                }

                ImGui::SameLine();
                if (ImGui::Button("Delete"))
                {
                    layers.Unregister_Layer(i);
                    editBuf[i][0] = '\0';
                }
            }

            ImGui::PopID();
        }
    }
    ImGui::EndChild();

    ImGui::Separator();

    // ====== 하단: Add Layer (ID 콤보 + 힌트 텍스트 + +버튼) ======
    static Layer::LAYER_ID s_selectedID = 0;
    static char s_newName[64] = "";

    std::vector<Layer::LAYER_ID> unused;
    layers.Gather_UnusedLayers(unused);

    // 1. ID 선택 (폭 50px로 축소)
    ImGui::SetNextItemWidth(50.f);
    const char* preview = (unused.empty() ? "--" : std::to_string((unsigned)s_selectedID).c_str());
    if (ImGui::BeginCombo("##UnusedID", preview))
    {
        for (auto id : unused)
        {
            if (ImGui::Selectable(std::to_string((unsigned)id).c_str(), id == s_selectedID))
                s_selectedID = id;
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();

    // 2. 이름 입력
    ImGui::SetNextItemWidth(295.f);
    ImGui::InputTextWithHint("##NewLayerName", "New Layer Name...", s_newName, IM_ARRAYSIZE(s_newName));

    ImGui::SameLine();

    // 3. + 버튼
    const bool canAdd = (!unused.empty() && s_newName[0] != '\0');
    ImGui::BeginDisabled(!canAdd);
    if (ImGui::Button("Add", ImVec2(120.f, 0)))
    {
        layers.Register_Layer(s_selectedID, s_newName);
        s_newName[0] = '\0'; // 입력창 비우기

        std::vector<Layer::LAYER_ID> nextUnused;
        layers.Gather_UnusedLayers(nextUnused);
        if (!nextUnused.empty()) s_selectedID = nextUnused.front();
    }
    ImGui::EndDisabled();

    ImGui::EndPopup();
}

void CInspectorPanel::Draw_RequiredComponent()
{
    if (!m_pTarget)
        return;
    Component::COMPONENT_MASK mask = m_pTarget->Get_ComponentMask();

    if ((m_pTarget->Get_ComponentMask() & Component::Component_Bit(COMPONENT_TYPE::TRANSFORM)) != 0)
        Draw_Transform();
    else if ((m_pTarget->Get_ComponentMask() & Component::Component_Bit(COMPONENT_TYPE::RECT_TRANSFORM)) != 0)
        Draw_RectTransform();
}

void CInspectorPanel::Draw_Components()
{
    if (!ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    Draw_CurrentComponents();

    ImVec2 size(ImGui::GetContentRegionAvail().x, 50.f);
    if (ImGui::Button("Add Component", size))
    {
        ImGui::OpenPopup("##AddComponentPopup");
    }

    Draw_AddComponentPopup();
    Draw_CreateScriptPopup();
}

void CInspectorPanel::Draw_CurrentComponents()
{
    Component::COMPONENT_MASK mask = m_pTarget->Get_ComponentMask();

    for (uint32_t i = 0; i < SCAST(uint32_t, COMPONENT_MAX); ++i)
    {
        if ((mask & Component::Component_Bit((COMPONENT_TYPE)i)) == 0)
            continue;

        COMPONENT_TYPE eType = (COMPONENT_TYPE)i;
        Draw_ComponentByType(eType);
    }

}


void CInspectorPanel::Draw_AddComponentPopup()
{
    if (!ImGui::BeginPopup("##AddComponentPopup"))
        return;

    if (ImGui::MenuItem("Transform"))
    {
        m_pTarget->Add_Component<CTransform>();
        ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("MeshRenderer"))
    {
        m_pTarget->Add_Component<CMeshRenderer>();
        ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("RectTransform"))
    {
        m_pTarget->Add_Component<CRectTransform>();
        ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("CanvasRenderer"))
    {
        m_pTarget->Add_Component<CCanvasRenderer>();
        ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Script"))
    {
        m_pTarget->Add_Component<CScript>();
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void CInspectorPanel::Draw_CreateScriptPopup()
{

}

void CInspectorPanel::Draw_Asset()
{
    const std::string name = m_selectedAsset.path.filename().string();
    const std::string path = m_selectedAsset.path.string();

    ImGui::TextUnformatted("Asset");
    ImGui::Separator();

    ImGui::Text("Name: %s", name.c_str());
    ImGui::Text("Type: %s", CAsset_Registry::AssetType_ToStr(m_selectedAsset.type));
    ImGui::Text("Path: %s", path.c_str());
    ImGui::Text("Directory: %s", m_selectedAsset.isDirectory ? "true" : "false");
}

void CInspectorPanel::Draw_None()
{
    ImGui::TextUnformatted("No selection.");
}


void CInspectorPanel::Draw_ComponentByType(COMPONENT_TYPE eComType)
{
    switch(eComType)
    {
    case COMPONENT_TYPE::MESH_RENDERER :
        Draw_MeshRenderer();
        break;
    case COMPONENT_TYPE::CANVAS_RENDERER:
        Draw_CanvasRenderer();
        break;
    case COMPONENT_TYPE::SCRIPT:
        Draw_Script();
        break;
    }
}

void CInspectorPanel::Draw_Transform()
{
    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    CTransform transform = m_pTarget->Get_Component<CTransform>();
    TRANSFORM_DATA* pData = transform._Data();

    /* Transform is not implemented yet */
    _float vPos[3] = { pData->vPosition.x, pData->vPosition.y, pData->vPosition.z };
    _float vScl[3] = { pData->vScale.x, pData->vScale.y, pData->vScale.z };
    _float3 f3Rot = transform.Get_Rotation_Euler();

    _float vRot[3] = { f3Rot.x, f3Rot.y, f3Rot.z };

    /* Need to call to read Transform data */
    _bool bChanged = false;

    ImGui::TextUnformatted("Position ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bChanged |= ImGui::DragFloat3("##Pos", vPos, 0.1f);

    ImGui::TextUnformatted("Rotation");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bChanged |= ImGui::DragFloat3("##Rot", vRot, 0.1f);

    ImGui::TextUnformatted("Scale     ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bChanged |= ImGui::DragFloat3("##Scl", vScl, 0.01f);

    if (bChanged)
    {
        pData->vPosition = { vPos[0], vPos[1], vPos[2] };
        pData->vScale = { vScl[0], vScl[1], vScl[2] };
        _float3 newEulerDeg{ vRot[0], vRot[1], vRot[2] };
        transform.Set_Rotation_Euler(newEulerDeg);

        pData->bDirty = true;
    }
}

void CInspectorPanel::Draw_RectTransform()
{
    if (!ImGui::CollapsingHeader("RectTransform", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    CRectTransform rt = m_pTarget->Get_Component<CRectTransform>();
    if (!rt.Is_Valid())
        return;

    RECTTRANSFORM_DATA* pData = rt._Data();
    if (!pData)
        return;


    _int vPos[2] = { SCAST(_int, pData->vPosPx.x),  SCAST(_int, pData->vPosPx.y) };
    _int vSize[2] = { SCAST(_int,pData->vSizePx.x), SCAST(_int,pData->vSizePx.y) };

    _bool bChanged = false;

    ImGui::TextUnformatted("Position");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bChanged |= ImGui::DragInt2("##UIPos", vPos, 1);

    ImGui::TextUnformatted("Size      ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bChanged |= ImGui::DragInt2("##UISize", vSize, 1);

    if (vSize[0] < 0) vSize[0] = 0;
    if (vSize[1] < 0) vSize[1] = 0;

    if (bChanged)
    {
        pData->vPosPx = { SCAST(_float, vPos[0]),  SCAST(_float, vPos[1]) };
        pData->vSizePx = { SCAST(_float, vSize[0]), SCAST(_float, vSize[1]) };
        pData->bDirty = true;
    }
}

void CInspectorPanel::Draw_MeshRenderer()
{
    CMeshRenderer mr = m_pTarget->Get_Component<CMeshRenderer>();
    if (!mr.Is_Valid())
        return;

    MESH_RENDERER_DATA* pData = mr._Data();
    if (!pData)
        return;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImGuiID idHeader = window->GetID("MeshRenderer_Header");
    const ImGuiID idCheck = window->GetID("MeshRenderer_Enable");

    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    _bool enabled = pData->bEnable;
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        SYS_COMPONENT.Set_Enable(COMPONENT_TYPE::MESH_RENDERER, mr.Get_Handle(), enabled);
        pData->bEnable = enabled;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("MeshRenderer", flags);

    ImGui::PopID();

    if (!open)
        return;

    bool bChanged = false;

    // --- Mesh ---
    ImGui::TextUnformatted("Mesh handle");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hMesh);

    // --- Material ---
    ImGui::TextUnformatted("Material handle");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hMaterial);

    // --- Layer ---
    int layer = (int)pData->layer;
    if (ImGui::DragInt("Layer", &layer, 1, 0, 10))
    {
        pData->layer = (RENDER_LAYER)layer;
        bChanged = true;
    }

    // --- Flags ---
    uint32_t flagsValue = pData->flags;
    if (ImGui::InputScalar("Flags", ImGuiDataType_U32, &flagsValue))
    {
        pData->flags = flagsValue;
        bChanged = true;
    }

    ImGui::TreePop();
}
void CInspectorPanel::Draw_CanvasRenderer()
{
    CCanvasRenderer cr = m_pTarget->Get_Component<CCanvasRenderer>();
    if (!cr.Is_Valid())
        return;

    CANVAS_RENDERER_DATA* pData = cr._Data();
    if (!pData)
        return;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImGuiID idHeader = window->GetID("CanvasRenderer_Header");
    const ImGuiID idCheck = window->GetID("CanvasRenderer_Enable");

    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    // 헤더 체크박스
    bool enabled = (pData->bEnable != 0);
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        pData->bEnable = enabled ? 1 : 0;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("CanvasRenderer", flags);

    ImGui::PopID();

    if (!open)
        return;

    bool bChanged = false;

    // (선택) 비활성화면 아래 UI 회색 + 입력 막기
    if (pData->bEnable == 0)
        ImGui::BeginDisabled();

    // Material
    ImGui::TextUnformatted("Material");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hMaterial);

    // Texture
    ImGui::TextUnformatted("Texture");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hTexture);

    // Layer
    int layer = (int)pData->layer;
    if (ImGui::DragInt("Layer", &layer, 1, 0, 10))
    {
        pData->layer = (RENDER_LAYER)layer;
        bChanged = true;
    }

    // Flags
    uint32_t flagsValue = pData->flags;
    if (ImGui::InputScalar("Flags", ImGuiDataType_U32, &flagsValue))
    {
        pData->flags = flagsValue;
        bChanged = true;
    }

    // SortZ
    float sortZ = pData->sortZ;
    if (ImGui::DragFloat("SortZ", &sortZ, 0.01f, 0.f, 1.f, "%.3f"))
    {
        if (sortZ < 0.f) sortZ = 0.f;
        if (sortZ > 1.f) sortZ = 1.f;
        pData->sortZ = sortZ;
        bChanged = true;
    }

    // Color
    _float4 col = pData->vColor;
    float c[4] = { col.x, col.y, col.z, col.w };
    if (ImGui::ColorEdit4("Color", c))
    {
        pData->vColor = { c[0], c[1], c[2], c[3] };
        bChanged = true;
    }

    // UV
    float uv[4] = { pData->rcUV.fLeft, pData->rcUV.fTop, pData->rcUV.fRight, pData->rcUV.fBottom };
    if (ImGui::DragFloat4("UV (L,T,R,B)", uv, 0.001f, 0.f, 1.f, "%.3f"))
    {
        for (int i = 0; i < 4; ++i)
        {
            if (uv[i] < 0.f) uv[i] = 0.f;
            if (uv[i] > 1.f) uv[i] = 1.f;
        }
        pData->rcUV = { uv[0], uv[1], uv[2], uv[3] };
        bChanged = true;
    }

    // Clip Rect
    bool clipEnabled = ((pData->flags & CF_CLIP_RECT) != 0);
    if (ImGui::Checkbox("Enable ClipRect", &clipEnabled))
    {
        if (clipEnabled) pData->flags |= CF_CLIP_RECT;
        else             pData->flags &= ~CF_CLIP_RECT;
        bChanged = true;
    }

    float clip[4] = { pData->rcClip.fLeft, pData->rcClip.fTop, pData->rcClip.fRight, pData->rcClip.fBottom };
    if (ImGui::DragFloat4("ClipRect (L,T,R,B)", clip, 1.f, 0.f, 0.f, "%.0f"))
    {
        pData->rcClip = { clip[0], clip[1], clip[2], clip[3] };
        bChanged = true;
    }

    if (pData->bEnable == 0)
        ImGui::EndDisabled();

    // if (bChanged)
    //     pData->bDirty = true;

    ImGui::TreePop();
}

void CInspectorPanel::Draw_Script()
{
    CScript sc = m_pTarget->Get_Component<CScript>();
    if (!sc.Is_Valid())
        return;

    SCRIPT_DATA* pData = sc._Data();
    if (!pData)
        return;

    // ------------------------------------------------------------
    // Header : [Enable Checkbox] + [Collapsible "Script"]
    // ------------------------------------------------------------
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImGuiID idHeader = window->GetID("Script_Header");
    const ImGuiID idCheck = window->GetID("Script_Enable");

    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    // 헤더 체크박스가 실제 enable을 담당
    _bool enabled = pData->bEnable;
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        m_pScript_Processor->Set_Enable(COMPONENT_TYPE::SCRIPT, sc.Get_Handle(), enabled);
        pData->bEnable = enabled;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("Script", flags);

    ImGui::PopID();

    if (!open)
        return;

    bool bChanged = false;

    /* TypeId, 런타임 인덱스 */
    ImGui::Text("TypeID: %u", (uint32_t)pData->iTypeID);

    // ------------------------------------------------------------
    // 캐시
    // ------------------------------------------------------------
    Engine::ASSET_GUID guid{};
    m_pScript_Processor->Try_Get_Guid_By_TypeID(pData->iTypeID, guid);

    const bool bHasBind = guid.Is_Valid();

    std::filesystem::path assetPath;
    const Engine::ASSET_RECORD* pRecord = nullptr;

    if (bHasBind)
    {
        assetPath = SYS_ASSET.Get_Asset_Path(guid);
        if (!assetPath.empty())
            pRecord = SYS_ASSET.Find(guid);
    }

    std::string boundClassName;
    std::string boundHeaderFileName;

    auto read_text_file_utf8 = [&](const std::filesystem::path& p, std::string& outText) -> bool
        {
            outText.clear();
            std::ifstream ifs(p, std::ios_base::binary);
            if (!ifs.is_open())
                return false;

            ifs.seekg(0, std::ios_base::end);
            const std::streamsize sz = ifs.tellg();
            if (sz <= 0)
                return false;
            ifs.seekg(0, std::ios_base::beg);

            outText.resize((size_t)sz);
            ifs.read(outText.data(), sz);
            return true;
        };

    auto try_parse_script_json = [&]()
        {
            if (!bHasBind) return;
            if (assetPath.empty()) return;
            if (!std::filesystem::exists(assetPath)) return;

            std::string txt;
            if (!read_text_file_utf8(assetPath, txt))
                return;

            try
            {
                json j = json::parse(txt);
                if (j.contains("ClassName") && j["ClassName"].is_string())
                    boundClassName = j["ClassName"].get<std::string>();
                if (j.contains("Header") && j["Header"].is_string())
                    boundHeaderFileName = j["Header"].get<std::string>();
            }
            catch (...)
            {
            }
        };

    /* 한 번만 파싱 */
    try_parse_script_json();

    /* 스크립트 바인딩 상태 */
    ImGui::TextUnformatted("Class");
    ImGui::SameLine();
    if (!bHasBind)
        ImGui::TextUnformatted("EMPTY");
    else
        ImGui::TextUnformatted(!boundClassName.empty() ? boundClassName.c_str() : "<unknown class>");

    // 파일 경로 표시 + "..." 버튼(열기/탐색기)
    if (bHasBind)
    {
        if (!assetPath.empty())
        {
            ImGui::SameLine();
            if (ImGui::SmallButton(".script"))
            {
                Editor_Util::RevealFile_In_Explorer(assetPath);
            }

            /* 바인딩 상태 표시 */
            ImGui::SameLine();
            if (ImGui::SmallButton("Open"))
            {
                /* 우선순위: Header(JSON의 Header) -> stem 기반 header/cpp -> 부모 폴더 */
                std::filesystem::path headerPath;
                std::filesystem::path cppPath;

                /* .script의 파일명(stem) */
                const std::string stem = assetPath.stem().string();

                const std::filesystem::path headerRoot = std::filesystem::path(Engine::ProjectConfig::CLIENT) / Engine::ProjectConfig::HEADER;
                const std::filesystem::path implRoot = std::filesystem::path(Engine::ProjectConfig::CLIENT) / Engine::ProjectConfig::IMPL;

                if (!boundHeaderFileName.empty())
                    headerPath = headerRoot / boundHeaderFileName;
                else
                    headerPath = headerRoot / (stem + ".h");

                cppPath = implRoot / (stem + ".cpp");

                auto try_open_or_reveal = [&](const std::filesystem::path& p) -> bool
                    {
                        if (p.empty() || !std::filesystem::exists(p))
                            return false;

                        Editor_Util::OpenFile_In_OS(p);
                        return true;
                    };

                bool bOpened = false;
                 if (try_open_or_reveal(cppPath)) bOpened = true;
                 else if (try_open_or_reveal(headerPath)) bOpened = true;

                if (!bOpened)
                {
                    /* 둘 다 없으면 .script 부모 폴더라도 열기 */
                    if (!assetPath.empty())
                        Editor_Util::RevealFile_In_Explorer(assetPath);
                    else
                        Editor_Util::RevealFile_In_Explorer(assetPath.parent_path());
                }
            }
        }
        else
        {
            ImGui::TextUnformatted("Asset Path: <not found>");
        }
    }

    // ------------------------------------------------------------
    // 드래그드롭 바인딩
    // ------------------------------------------------------------
    ImGui::Separator();
    ImGui::TextUnformatted("Bind Script");
    ImGui::SameLine();
    ImGui::TextDisabled(bHasBind ? "(Already bound)" : "(Drag script asset here)");

    ImVec2 dropSize(ImGui::GetContentRegionAvail().x, 28.f);

    /* 이미 바인딩됐으면 드롭 타겟 자체를 비활성화 */
    if (bHasBind)
        ImGui::BeginDisabled();

    const char* dropLabel = bHasBind
        ? (!boundClassName.empty() ? boundClassName.c_str() : "BOUND")
        : "EMPTY";

    ImGui::Button(dropLabel, dropSize);

    if (!bHasBind)
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("ASSET_GUID"))
            {
                const Engine::ASSET_GUID* pGUID = (const Engine::ASSET_GUID*)p->Data;
                if (pGUID && pGUID->Is_Valid())
                {
                    auto pRec = SYS_ASSET.Find(*pGUID);
                    if (pRec && pRec->eType == Engine::ASSET_TYPE::SCRIPT)
                    {
                        // 여기서만 바인딩
                        m_pScript_Processor->Rebind_ScriptGuid(sc.Get_Handle(), *pGUID);

                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    if (bHasBind)
        ImGui::EndDisabled();

    ImGui::TreePop();
}

std::unique_ptr<CInspectorPanel> CInspectorPanel::Create(const std::string& strPanelName, CHierarchyPanel* pHierarcy, CProjectPanel* pProject)
{
    auto pInstance = std::make_unique<CInspectorPanel>(strPanelName);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(pHierarcy, pProject), nullptr, "CInspectorPanel Create failed");
    return pInstance;
}

NS_END
