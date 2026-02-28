#pragma once
#include "Engine_Define.h"

namespace StringUtils
{
    /* path to utf8 */
    inline std::string Path_To_UTF8(const std::filesystem::path& path)
    {
        auto u8Str = path.u8string();
        return std::string(u8Str.begin(), u8Str.end());
    }

    /* wstring -> utf8 string */
    inline std::string WString_To_UTF8(const std::wstring& wstr)
    {
        if (wstr.empty()) return "";

        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);

        std::string res(sizeNeeded, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &res[0], sizeNeeded, NULL, NULL);

        return res;
    }
}
