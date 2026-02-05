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
}
