#pragma once

#include "Editor_Define.h"

NS_BEGIN(Editor)

class CCreate_Asset_Helper
{
public: /* ---------- Script ---------- */
    static void Build_Script_Source(
        const std::string& className,
        const std::filesystem::path& headerPath,
        std::string& outHeader,
        std::string& outCpp
    );


    static _bool Write_Script_Source_Files(
        const std::filesystem::path& headerPath,
        const std::filesystem::path& cppPath,
        const std::string& header,
        const std::string& cpp
    );

    static _bool Write_Script_Asset_File(
        const std::filesystem::path& savePath,
        const std::string& className,
        const std::filesystem::path& headerPath
    );

    static std::string Make_Script_File_From_Stem(const std::string& stem);
    static std::string Make_Unique_File_Stem_Impl(
        const std::filesystem::path& parent,
        const std::string& baseStem, 
        const std::filesystem::path& headerPath,
        const std::filesystem::path& cppPath);



public : /* ---------- Common ---------- */
    static _bool Write_Text_File(const std::filesystem::path& p, const std::string& utf8);


};

NS_END
