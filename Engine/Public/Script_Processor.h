#pragma once
#include "Component_Processor_Impl.h"
#include "Script.h"

NS_BEGIN(Engine)
class CScript_Handler;

typedef struct tagScriptTypeInfo
{
    SCRIPT_VTABLE vt{};
}SCRIPT_TYPE_INFO;

class ENGINE_DLL CScript_Processor : public CComponent_Processor_Impl<CScript, COMPONENT_TYPE::SCRIPT>
{
    DEF_PROCESSOR_ID(PROCESSOR_ID::SCRIPT)

public:
    HRESULT Initialize() override;
    void Update(_float fDT) override;
    void LateUpdate(_float fDT) override;

    HRESULT Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec) override;
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;

    void Set_Enable(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, _bool bEnable) override;

public:
    /* 각 컴포넌트에 필요한 로직들 */
    _bool   Try_Get_Guid_By_TypeID(TypeID typeId, ASSET_GUID& out) const;
    HRESULT Rebind_ScriptGuid(COMPONENT_HANDLE hScript, const ASSET_GUID& guid);

    IScript* Get_Script_Instance(COMPONENT_HANDLE hScript);

private :
    CScript_Handler* m_pScript_Handler{ };
    typedef struct tagTickCall
    {
        COMPONENT_HANDLE    hScript{};
        ScriptTickFn        fn{};
    }TICK_CALL;

    typedef struct tagPendingScript
    {
        COMPONENT_HANDLE    hScript{};
        size_t              iTypeIndex;
    } PENDING_SCRIPT;

    std::vector<SCRIPT_TYPE_INFO>   m_Types;
    std::vector<TICK_CALL>          m_Ticks[SCAST(uint8_t, SCRIPT_TICK::END)];
    SCRIPT_CTX m_ctx{};

    std::vector<PENDING_SCRIPT>             m_PendingAwake_Script; /* 에디터 런타임 등록 시, 즉시 Awake 실행 되는 문제 */

    std::unordered_map<ASSET_GUID, TypeID, ASSET_GUID_HASHER>   m_GuidToTypeID;
    std::vector<ASSET_GUID>                                     m_TypeIDToGUID;

private :
    void Initialize_Component_Data(COMPONENT_HANDLE hComponent) override;

    void Ensure_Awake(SCRIPT_DATA& tData, SCRIPT_TYPE_INFO& tTypeInfo);
    void Ensure_Start(SCRIPT_DATA& tData, SCRIPT_TYPE_INFO& tTypeInfo);
    void Add_To_TickLists(COMPONENT_HANDLE hScript, const SCRIPT_DATA& tData);
    void Remove_From_TickLists(COMPONENT_HANDLE hScript);

    TypeID Find_Or_Create(const ASSET_GUID& tGUID);
    void Create_State_If_Needed(COMPONENT_HANDLE hScript, SCRIPT_DATA* pData);
    void Reset_Data(COMPONENT_HANDLE hScript, SCRIPT_DATA* pData);
    void Reset_Data_On_Deallocate(COMPONENT_HANDLE hScript, SCRIPT_DATA* pData);
    void Flush_PendingAwake();

public:
    static std::unique_ptr<CScript_Processor> Create();

};

NS_END
