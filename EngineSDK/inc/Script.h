#pragma once
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)
using TypeID = uint32_t;

typedef struct tagScriptCtx
{
    /* 일단 비워둠 */
}SCRIPT_CTX;

enum class SCRIPT_TICK : uint8_t
{
    PRIORITY = 0,
    UPDATE = 1,
    LATE = 2,
    END
};

using ScriptTickFn = void(*)(void* pState, SCRIPT_CTX& tCTX, _float fDT);

typedef struct tagScriptVTable
{
    /* state lifetime */
    void* (*Create)();          // new StateT
    void  (*Destroy)(void*);    // delete StateT

    /* lifecycle */
    void (*Awake)(void* pState, SCRIPT_CTX& ctx);
    void (*Start)(void* pState, SCRIPT_CTX& ctx);

    /* update stages */
    ScriptTickFn Tick[SCAST(uint8_t, SCRIPT_TICK::END)];
}SCRIPT_VTABLE;

enum : uint8_t
{
    SCRIPT_FLAG_AWOKEN = 1 << 0,
    SCRIPT_FLAG_STARTED = 1 << 1,
};

typedef struct tagScriptData
{
    OBJECT_HANDLE   hObject{};
    _bool           bEnable = false;

    void* pState{};

    uint32_t        hState = INVALID_HANDLE_UINT; // 0
    TypeID          iTypeID = 0;                   // 0 = invalid

    uint8_t         iFlags = 0;                    // bit0 enabled, bit1 awoken, bit2 started
    uint8_t         pad[3] = {};
} SCRIPT_DATA;

class ENGINE_DLL CScript : public CComponent_Proxy_Base<SCRIPT_DATA, CScript, COMPONENT_TYPE::SCRIPT>
{
public:
    static constexpr COMPONENT_TYPE kComponentType = COMPONENT_TYPE::SCRIPT;

    CScript() : CComponent_Proxy_Base() {}
    CScript(DataType* pData, COMPONENT_HANDLE handle) : CComponent_Proxy_Base(pData, handle) {}
    ~CScript() override = default;
};


class ENGINE_DLL IScript
{
public:
    virtual ~IScript() = default;

    virtual void Awake(void* pCtx) { (void)pCtx; }
    virtual void Start(void* pCtx) { (void)pCtx; }

    virtual void Priority_Update(void* pCtx, _float fDT) { (void)pCtx; (void)fDT; }
    virtual void Update(void* pCtx, _float fDT) { (void)pCtx; (void)fDT; }
    virtual void Late_Update(void* pCtx, _float fDT) { (void)pCtx; (void)fDT; }
};


NS_END
