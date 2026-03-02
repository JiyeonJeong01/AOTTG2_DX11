#include "Script_Processor.h"
#include "Component_System.h"
#include "Component_Spec.h"
#include "Asset_Registry.h"
#include "Script_Handler.h"

HRESULT CScript_Processor::Initialize()
{
    SYS_COMPONENT.Register_InitialSpecFactory<CScript, SCRIPT_DATA>(COMPONENT_TYPE::SCRIPT);
    SYS_COMPONENT.Register_BuildSpecFacotry<CScript>(COMPONENT_TYPE::SCRIPT);

    m_Pool.Subscribe_OnDeallocate(&CScript_Processor::Reset_Data_On_Deallocate, this);

    if (m_Types.empty()) m_Types.resize(1); // typeId=0 invalid
    return S_OK;
}

void CScript_Processor::Update(_float fDT)
{
    /* Priority_Update */
    {
        auto& ticks = m_Ticks[SCAST(uint8_t, SCRIPT_TICK::PRIORITY)];
        for (size_t i = 0; i < ticks.size(); ++i)
        {
            const auto h = ticks[i].hScript;
            SCRIPT_DATA* pData = m_Pool.Get_Data_By_Handle(h);
            if (!pData) continue;

            if ((pData->iFlags & SCRIPT_FLAG_ENABLED) == 0)
                continue;

            const uint32_t iTypeID = pData->iTypeID;
            if (iTypeID == 0 || iTypeID >= m_Types.size())
                continue;

            Ensure_Awake(*pData, m_Types[iTypeID]);
            Ensure_Start(*pData, m_Types[iTypeID]);

            if (ticks[i].fn)
                ticks[i].fn(pData->pState, m_ctx, fDT);
        }
    }

    /* UPDATE */
    {
        auto& ticks = m_Ticks[SCAST(uint8_t, SCRIPT_TICK::UPDATE)];
        for (size_t i = 0; i < ticks.size(); ++i)
        {
            const auto h = ticks[i].hScript;
            SCRIPT_DATA* pData = m_Pool.Get_Data_By_Handle(h);
            if (!pData) continue;

            if ((pData->iFlags & SCRIPT_FLAG_ENABLED) == 0) continue;

            /* TODO Priority를 통과했다면 여기서부터는 지워도 될 거 같은데 확인해보기 */
            const uint32_t typeId = pData->iTypeID;
            if (typeId == 0 || typeId >= m_Types.size()) continue;

            if (ticks[i].fn)
                ticks[i].fn(pData->pState, m_ctx, fDT);
        }
    }

    /* LATE_UPDATE */
    auto& ticks = m_Ticks[SCAST(uint8_t, SCRIPT_TICK::LATE)];
    for (size_t i = 0; i < ticks.size(); ++i)
    {
        const auto h = ticks[i].hScript;
        SCRIPT_DATA* pData = m_Pool.Get_Data_By_Handle(h);
        if (!pData) continue;

        if ((pData->iFlags & SCRIPT_FLAG_ENABLED) == 0) continue;

        const uint32_t typeId = pData->iTypeID;
        if (typeId == 0 || typeId >= m_Types.size()) continue;

        if (ticks[i].fn)
            ticks[i].fn(pData->pState, m_ctx, fDT);
    }
}

void CScript_Processor::LateUpdate(_float fDT)
{
    // TODO 전체 프로세서에서 얘 삭제 
}

HRESULT CScript_Processor::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec)
{
    IF_NULL_RETURN_MSG_BREAK(pSpec, E_FAIL, "Script spec is nullptr.");

    auto* pData = m_Pool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "Script data is nullptr.");

    const auto* spec = SCAST(const SCRIPT_SPEC*, pSpec);

    TypeID iTypeID = Find_Or_Create(spec->scriptGuid); /* 해당 TypeId 없다면 새 슬롯 생성 */

    /* vtable은 빌드 전이면 없을 수 있음: 이 경우 실패하지 말고 바인딩만 유지 */
    const SCRIPT_VTABLE* pVt = SYS_ASSET.Scripts().Find(spec->scriptGuid);
    if (!pVt)
    {
        m_Types[iTypeID].vt = SCRIPT_VTABLE{};
        return S_OK;
    }


    if (m_Types.size() <= iTypeID)
        m_Types.resize((size_t)iTypeID + 1);

    m_Types[iTypeID].vt = *pVt; /* 슬롯에 넣기 */

    pData->iTypeID = iTypeID;
    pData->iFlags = 0;
    if (spec->bEnabled)
        pData->iFlags |= SCRIPT_FLAG_ENABLED;

    Create_State_If_Needed(hComponent, pData);

    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CScript_Processor::Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    IF_TRUE_RETURN_MSG_BREAK(eComType != COMPONENT_TYPE::SCRIPT, nullptr, "Wrong access to processor");

    auto* pData = m_Pool.Get_Data_By_Handle(hComponent);
    if (!pData)
        return nullptr;

    ASSET_GUID tGUID{};
    for (const auto& it : m_GuidToTypeID)
    {
        if (it.second == pData->iTypeID)
            tGUID = it.first;
    }

    auto out = std::make_unique<SCRIPT_SPEC>();

    out->scriptGuid = tGUID;
    out->bEnabled = ((pData->iFlags & SCRIPT_FLAG_ENABLED) != 0) ? 1 : 0;

    return out;
}

void CScript_Processor::Set_Enabled(COMPONENT_HANDLE hComponent, _bool bEnable)
{
    auto* pData = m_Pool.Get_Data_By_Handle(hComponent);
    if (!pData)
        return;

    const _bool wasEnabled = ((pData->iFlags & SCRIPT_FLAG_ENABLED) != 0);
    const _bool nowEnabled = (bEnable != 0);

    if (wasEnabled == nowEnabled)
        return;

    if (nowEnabled)
    {
        pData->iFlags |= SCRIPT_FLAG_ENABLED;

        if (!pData->pState)
            Create_State_If_Needed(hComponent, pData);

        Add_To_TickLists(hComponent, *pData);
    }
    else
    {
        pData->iFlags &= ~SCRIPT_FLAG_ENABLED;
        Remove_From_TickLists(hComponent);
    }
}

/* Editor에서 Script 컴포넌트를 '추가'한 경우, 아직 Initialize_From_Spec 경로를 타지 않아서 pData->iTypeID == 0(Invalid) 상태로 남는다.
 * 이 상태로 저장(Build_Spec)하면 scriptGuid가 invalid로 기록되어, 로드 시 vtable 조회가 실패할 수 있다.
 * 따라서 Editor에서는 Script 컴포넌트 추가 직후, 반드시 유효한 scriptGuid를 지정하기 위해
 * Rebind_ScriptGuid를 호출해야 한다. (타입 바인딩 -> state 생성/등록)  */
HRESULT CScript_Processor::Rebind_ScriptGuid(COMPONENT_HANDLE hScript, const ASSET_GUID& tGUID)
{
    auto* pData = m_Pool.Get_Data_By_Handle(hScript);
    IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "Script data is nullptr.");

    /* 기존 Tick과 State 제거 */
    Reset_Data(hScript, pData);

    /* 타입이 바뀌면 무조건 Start 다시 호출하도록 한다. */
    pData->iFlags &= ~SCRIPT_FLAG_AWOKEN;
    pData->iFlags &= ~SCRIPT_FLAG_STARTED;

    /* GUID로 TypeID 슬롯 확보한다. */
    TypeID typeId = Find_Or_Create(tGUID);
    pData->iTypeID = typeId;
    if (typeId >= m_Types.size())
        m_Types.resize(typeId + 1);

    /* GUID로 등록된 VTable을 찾아 할당한다. */
    const SCRIPT_VTABLE* pVt = SYS_ASSET.Scripts().Find(tGUID);

    /* 아직 빌드 전이라 vtable이 없을 수 있다. GUID 바인딩만 유지하고, state/tick 등록은 스킵한다. */
    if (!pVt)
    {
        m_Types[typeId].vt = SCRIPT_VTABLE{};
        return S_OK;
    }

    m_Types[typeId].vt = *pVt;

    /* vt.Create로 pState 생성한 뒤, Enable하다면 TickList에 넣는다. */
    Create_State_If_Needed(hScript, pData);

    return S_OK;
}

_bool CScript_Processor::Try_Get_Guid_By_TypeID(TypeID typeId, ASSET_GUID& out) const
{
    out = ASSET_GUID{};

    if (typeId == 0) return false;

    if (SCAST(size_t, typeId) >= m_TypeIDToGUID.size())
        return false;

    out = m_TypeIDToGUID[typeId];
    return out.Is_Valid();
}

/* Initialzie_Componet_Spec 에서 바로 Spec으로 값 세팅 가능하게 데이터를 초기값으로 세팅/정리한다.
 * Script 컴포넌트의 경우 추가 이후 반드시 재빌드가 필요하고 그 때 Initialzie_Componet_Spec을 통해 적절한 데이터를 받아야 한다. */
void CScript_Processor::Initialize_Component_Data(COMPONENT_HANDLE hComponent)
{
    SCRIPT_DATA* pData = m_Pool.Get_Data_By_Handle(hComponent);
    if (!pData)
        return;

    // Allocate 직후 초기값만 세팅 (정리는 Deallocate에서만)
    pData->pState = nullptr;
    pData->iTypeID = 0;
    pData->iFlags |= SCRIPT_FLAG_ENABLED; /* TODO ::::::::::::::::::::::::::::: 이거 false여야 하려나?*/
}

void CScript_Processor::Ensure_Awake(SCRIPT_DATA& tData, SCRIPT_TYPE_INFO& tTypeInfo)
{
    if ((tData.iFlags & SCRIPT_FLAG_AWOKEN) != 0)
        return;
    if (!tData.pState)
        return;

    if (tTypeInfo.vt.Awake)
        tTypeInfo.vt.Awake(tData.pState, m_ctx);

    tData.iFlags |= SCRIPT_FLAG_AWOKEN;
}

void CScript_Processor::Ensure_Start(SCRIPT_DATA& tData, SCRIPT_TYPE_INFO& tTypeInfo)
{
    if ((tData.iFlags & SCRIPT_FLAG_STARTED) != 0)
        return;
    if (!tData.pState)
        return;

    if (tTypeInfo.vt.Start)
        tTypeInfo.vt.Start(tData.pState, m_ctx);
    tData.iFlags |= SCRIPT_FLAG_STARTED;
}

void CScript_Processor::Add_To_TickLists(COMPONENT_HANDLE hScript, const SCRIPT_DATA& tData)
{
    if ((tData.iFlags & SCRIPT_FLAG_ENABLED) == 0)
        return;

    if (!tData.pState)
        return;

    uint32_t iTypeID = tData.iTypeID;
    if (iTypeID == 0 || iTypeID >= m_Types.size())
        return;

    const auto& vt = m_Types[iTypeID].vt;
    for (uint8_t i = 0; i < SCAST(uint8_t, SCRIPT_TICK::END); ++i)
    {
        if (!vt.Tick[i])
            continue;
        m_Ticks[i].push_back(TICK_CALL{ hScript, vt.Tick[i] });
    }
}

void CScript_Processor::Remove_From_TickLists(COMPONENT_HANDLE hScript)
{
    for (uint8_t i = 0; i < SCAST(uint8_t, SCRIPT_TICK::END); ++i)
    {
        auto& v = m_Ticks[i];
        for (size_t j = 0; j < v.size(); ++j)
        {
            if (v[j].hScript == hScript)
            {
                v[j] = v.back();
                v.pop_back();
                break;
            }
        }
    }
}

TypeID CScript_Processor::Find_Or_Create(const ASSET_GUID& tGUID)
{
    auto it = m_GuidToTypeID.find(tGUID);
    if (it != m_GuidToTypeID.end())
        return it->second;

    TypeID newId = SCAST(TypeID, m_Types.size());
    m_Types.push_back(SCRIPT_TYPE_INFO{});   // 실제 슬롯 생성

    m_GuidToTypeID.emplace(tGUID, newId);

    if (newId >= m_TypeIDToGUID.size())
        m_TypeIDToGUID.resize((size_t)newId + 1);
    m_TypeIDToGUID[newId] = tGUID;
    return newId;
}

void CScript_Processor::Create_State_If_Needed(COMPONENT_HANDLE hScript, SCRIPT_DATA* pData)
{
    const uint32_t typeId = pData->iTypeID;
    if (typeId == 0 || typeId >= m_Types.size()) return;

    auto& vt = m_Types[typeId].vt;
    if (!vt.Create) return;

    if (!pData->pState)
    {
        pData->pState = vt.Create();
        if (vt.Awake)
            vt.Awake(pData->pState, m_ctx);
    }

    /* Enable한 스크립트면 다시 등록한다. */
    if ((pData->iFlags & SCRIPT_FLAG_ENABLED) != 0)
        Add_To_TickLists(hScript, *pData);
}

void CScript_Processor::Reset_Data(COMPONENT_HANDLE hScript, SCRIPT_DATA* pData)
{
    Remove_From_TickLists(hScript);

    if (pData->pState)
    {
        const uint32_t oldTypeId = pData->iTypeID;
        if (oldTypeId != 0 && oldTypeId < m_Types.size())
        {
            auto& oldVt = m_Types[oldTypeId].vt;
            if (oldVt.Destroy)
                oldVt.Destroy(pData->pState);
        }
        pData->pState = nullptr;
    }
}

void CScript_Processor::Reset_Data_On_Deallocate(COMPONENT_HANDLE hScript, SCRIPT_DATA* pData)
{
    if (!pData) return;

    Reset_Data(hScript, pData);

    pData->iTypeID = 0;
    pData->iFlags = 0;
}


std::unique_ptr<CScript_Processor> CScript_Processor::Create()
{
    auto pInstance = std::make_unique<CScript_Processor>();

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}
