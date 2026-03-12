#pragma once

#include "Converter_Define.h"

NS_BEGIN(Converter)

static inline void CopyFloat3(_float3& dst, const aiVector3D& src)
{
    dst.x = src.x; dst.y = src.y; dst.z = src.z;
}
static inline void CopyFloat2(_float2& dst, const aiVector3D& src)
{
    dst.x = src.x; dst.y = src.y;
}

static std::string To_String_Utf8(const GUID& value)
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

NS_END
