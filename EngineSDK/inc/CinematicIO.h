#pragma once
#include "Cinematic_Define.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCinematicIO final
{
public:
    static bool Save(const std::string& strFileName, const CINEMATIC_CLIP& tClip);
    static bool Load(const std::string& strFileName, CINEMATIC_CLIP& tOutClip);
};

NS_END
