// Script_Register.h
#pragma once

#include "Asset_Registry.h"
#include "Script.h"
#include "Script_Handler.h"

NS_BEGIN(Engine)

template <typename TScript>
static void Script_Awake(void* pState, SCRIPT_CTX& tCTX)
{
    static_cast<TScript*>(pState)->Awake(&tCTX);
}

template <typename TScript>
static void Script_Start(void* pState, SCRIPT_CTX& tCTX)
{
    static_cast<TScript*>(pState)->Start(&tCTX);
}

template <typename TScript>
static void Script_Tick_Priority(void* pState, SCRIPT_CTX& tCTX, _float fDT)
{
    static_cast<TScript*>(pState)->Priority_Update(&tCTX, fDT);
}

template <typename TScript>
static void Script_Tick_Update(void* pState, SCRIPT_CTX& tCTX, _float fDT)
{
    static_cast<TScript*>(pState)->Update(&tCTX, fDT);
}

template <typename TScript>
static void Script_Tick_Late(void* pState, SCRIPT_CTX& tCTX, _float fDT)
{
    static_cast<TScript*>(pState)->Late_Update(&tCTX, fDT);
}

template <typename TScript>
static void* Script_Create()
{
    return new TScript{};
}

template <typename TScript>
static void Script_Destroy(void* pState)
{
    delete static_cast<TScript*>(pState);
}

template <typename TScript>
static void Fill_VTable_Default(SCRIPT_VTABLE& vt)
{
    vt.Create = &Script_Create<TScript>;
    vt.Destroy = &Script_Destroy<TScript>;

    vt.Awake = &Script_Awake<TScript>;
    vt.Start = &Script_Start<TScript>;

    vt.Tick[SCAST(uint8_t, SCRIPT_TICK::PRIORITY)] = &Script_Tick_Priority<TScript>;
    vt.Tick[SCAST(uint8_t, SCRIPT_TICK::UPDATE)] = &Script_Tick_Update<TScript>;
    vt.Tick[SCAST(uint8_t, SCRIPT_TICK::LATE)] = &Script_Tick_Late<TScript>;
}

template <typename TScript>
struct ScriptBinder
{
    static Engine::SCRIPT_VTABLE Build()
    {
        Engine::SCRIPT_VTABLE vt{};
        Engine::Fill_VTable_Default<TScript>(vt);
        return vt;
    }
};

NS_END
