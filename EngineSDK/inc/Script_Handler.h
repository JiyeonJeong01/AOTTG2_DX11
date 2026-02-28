#pragma once
#include <unordered_map>
#include <vector>
#include "Script.h"

NS_BEGIN(Engine)

class CAsset_Registry;

class ENGINE_DLL CScript_Handler final
{
public:
    CScript_Handler();
    ~CScript_Handler();
public:
    void Clear();
    void Register_All_();
    void Cache_GUID(const ASSET_GUID& tGUID);
    void Register_VTable(const ASSET_GUID& tGUID, const SCRIPT_VTABLE& vt);
    const SCRIPT_VTABLE* Find(const ASSET_GUID& tGUID) const;

    static HRESULT Generate(const std::filesystem::path& outCppPath);

private:
    static std::string To_Include_Line(const std::string& className);
    static std::string Escape_Cpp_String(const std::string& s);

private:
    static std::vector<ASSET_GUID>                                      m_CachedGUID;
    std::unordered_map<ASSET_GUID, SCRIPT_VTABLE, ASSET_GUID_HASHER>    m_VTableMap;

public :
    static std::unique_ptr<CScript_Handler> Create();
};

NS_END
