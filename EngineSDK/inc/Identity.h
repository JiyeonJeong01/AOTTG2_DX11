#pragma once

#include "Engine_Define.h"

#pragma comment (lib, "ole32.lib")

NS_BEGIN(Engine)

typedef struct tagAssetGUID
{
    GUID value{};

    _bool Is_Valid() const
    {
        return !IsEqualGUID(value, GUID_NULL);
    }

    _bool operator==(const tagAssetGUID& rhs) const
    {
        return InlineIsEqualGUID(value, rhs.value) != 0;
    }

    _bool operator!=(const tagAssetGUID& rhs) const
    {
        return !(*this == rhs);
    }

    static tagAssetGUID New_GUID()
    {
        tagAssetGUID g{};
        ::CoCreateGuid(&g.value);
        return g;
    }

    std::string To_String_Utf8() const
    {
        wchar_t szBuff[64]{};

        /* Convert GUID to wide string(standard Windows format : {XXXXXXXX - XXXX - ...}) */
        _int n = ::StringFromGUID2(value, szBuff, 64);
        if (n <= 0)
            return {};
        /* Calculate required buffer size for UTF-8 conversion */
        _int iLen = ::WideCharToMultiByte(CP_UTF8, 0, szBuff, -1, nullptr, 0, nullptr, nullptr);
        std::string strOut;
        strOut.resize((iLen > 0) ? (iLen - 1) : 0);

        /* Convert Wide string to Multi-byte string (UTF-8) */
        if (!strOut.empty())
            ::WideCharToMultiByte(CP_UTF8, 0, szBuff, -1, strOut.data(), iLen, nullptr, nullptr);

        if (!strOut.empty() && strOut.front() == '{' && strOut.back() == '}')
            strOut = strOut.substr(1, strOut.size() - 2);

        return strOut;
    }

    static _bool Try_Utf8_To_GUID(const std::string& sUtf8, tagAssetGUID& out)
    {
        /* Accept with/without braces */
        std::string strTmp = sUtf8;
        if (!strTmp.empty() && strTmp.front() != '{')
            strTmp = "{" + strTmp + "}";

        /* Convert UTF-8 string to UTF-16 (WideChar) */ 
        _int wlen = ::MultiByteToWideChar(CP_UTF8, 0, strTmp.c_str(), -1, nullptr, 0);
        if (wlen <= 0)
            return false;

        std::wstring wStr;
        wStr.resize(wlen - 1);
        ::MultiByteToWideChar(CP_UTF8, 0, strTmp.c_str(), -1, wStr.data(), wlen);

        /* Parse the wide string into a native GUID structure */
        GUID g{};
        HRESULT hr = ::CLSIDFromString(wStr.c_str(), &g);
        if (FAILED(hr))
            return false;

        out.value = g;
        return true;
    }

    tagAssetGUID() : value(GUID_NULL) {};

    tagAssetGUID(const std::string& str)
    {
        value = GUID_NULL;
        Try_Utf8_To_GUID(str, *this);
    }
}ASSET_GUID;

typedef struct tagInstanceUUID
{
    GUID value{};

    static tagInstanceUUID New()
    {
        tagInstanceUUID id;
        if (FAILED(::CoCreateGuid(&id.value)))
            id.value = GUID_NULL;
        return id;
    }

    _bool Is_Valid() const
    {
        return !IsEqualGUID(value, GUID_NULL);
    }

    _bool operator==(const tagInstanceUUID& rhs) const
    {
        return InlineIsEqualGUID(value, rhs.value) != 0;
    }

    _bool operator!=(const tagInstanceUUID& rhs) const
    {
        return !(*this == rhs);
    }

    std::string To_String_Utf8() const
    {
        wchar_t szBuff[64]{};

        int n = ::StringFromGUID2(value, szBuff, 64);
        if (n <= 0)
            return {};

        int iLen = ::WideCharToMultiByte(CP_UTF8, 0, szBuff, -1, nullptr, 0, nullptr, nullptr);
        std::string out;
        out.resize((iLen > 0) ? (iLen - 1) : 0);

        if (!out.empty())
            ::WideCharToMultiByte(CP_UTF8, 0, szBuff, -1, out.data(), iLen, nullptr, nullptr);

        if (!out.empty() && out.front() == '{' && out.back() == '}')
            out = out.substr(1, out.size() - 2);

        return out;
    }

    static _bool Try_Utf8_To_UUID(const std::string& sUtf8, tagInstanceUUID& out)
    {
        std::string tmp = sUtf8;
        if (tmp.empty())
            return false;

        if (tmp.front() != '{')
            tmp = "{" + tmp + "}";

        int wlen = ::MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), -1, nullptr, 0);
        if (wlen <= 0)
            return false;

        std::wstring wStr;
        wStr.resize(wlen - 1);
        ::MultiByteToWideChar(CP_UTF8, 0, tmp.c_str(), -1, wStr.data(), wlen);

        GUID g{};
        HRESULT hr = ::CLSIDFromString(wStr.c_str(), &g);
        if (FAILED(hr))
            return false;

        out.value = g;
        return true;
    }
}INSTANCE_UUID;

typedef struct tagAssetGuidHasher
{
    size_t operator()(const ASSET_GUID& g) const noexcept
    {
        /* Treat 128-bit GUID as two 64-bit integers for hashing */ 
        const uint64_t* p = reinterpret_cast<const uint64_t*>(&g.value);
        uint64_t a = p[0];
        uint64_t b = p[1];

        /* Mix the bits to create a unique hash (similar to boost::hash_combine) */
        a ^= b + 0x9e3779b97f4a7c15ull + (a << 6) + (a >> 2);
        return SCAST(size_t, a);
    }
}ASSET_GUID_HASHER;

typedef struct tagInstanceUuidHasher
{
    size_t operator()(const INSTANCE_UUID& i) const noexcept
    {
        /* Treat 128-bit GUID as two 64-bit integers for hashing */ 
        const uint64_t* p = reinterpret_cast<const uint64_t*>(&i.value);
        uint64_t a = p[0];
        uint64_t b = p[1];

        /* Mix the bits to create a unique hash (similar to boost::hash_combine) */
        a ^= b + 0x9e3779b97f4a7c15ull + (a << 6) + (a >> 2);
        return SCAST(size_t, a);
    }
}INSTANCE_UUID_HASHER;

typedef struct ENGINE_DLL tagAssetRecord
{
    ASSET_GUID              tGUID{};
    ASSET_TYPE              eType{ ASSET_TYPE::UNKNOWN };
    ASSET_SRC               eSrc{ ASSET_SRC::FILE };

    std::filesystem::path   path{};
    _bool                   bDirectory{ false };

    tagAssetRecord() = default;
    tagAssetRecord(ASSET_GUID g, ASSET_TYPE t, ASSET_SRC eSrc, std::filesystem::path p, _bool b)
        : tGUID(std::move(g)), eType(std::move(t)), eSrc(eSrc), path(std::move(p)), bDirectory(b) {}
}ASSET_RECORD;

typedef struct ENGINE_DLL DefaultAssetGuid
{
    static inline ASSET_GUID MESH_CUBE{ "11111111-1111-1111-1111-111111111111" };
    static inline ASSET_GUID MESH_RECT{ "22222222-2222-2222-2222-222222222222" };
    static inline ASSET_GUID MESH_SPHERE{ "33333333-3333-3333-3333-333333333333" };
    static inline ASSET_GUID MATERIAL{ "44444444-4444-4444-4444-444444444444" };
    static inline ASSET_GUID SHADER_VTXCOL { "FA9F00D0-9F1B-4014-A3FA-AAA5A31F7946" };
    static inline ASSET_GUID SHADER_VTXTEX { "3EE2D609-436B-4D16-8AD5-FB1D1185D44C" };
}DEFAULT_ASSET_GUID;


NS_END
