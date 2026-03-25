#pragma once
#include "Engine_Define.h"
#include "Identity.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagFontEntry
{
    ASSET_GUID tGUID{};

    std::unique_ptr<DirectX::SpriteFont> pFont;

public:
    _bool Is_Valid() const noexcept
    {
        return pFont != nullptr;
    }

} FONT_ENTRY;

NS_END
