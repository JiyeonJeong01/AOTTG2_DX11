#pragma once
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagSpriteEffectData final
{
    OBJECT_HANDLE       hObject{};
    _bool               bEnable = true;

    COMPONENT_HANDLE    hTransform = INVALID_HANDLE;

    uint32_t            hTexture = INVALID_HANDLE_UINT;
    uint32_t            hMaterial = INVALID_HANDLE_UINT;
    uint32_t            hPerObjectParams = INVALID_HANDLE_UINT;

    RENDER_LAYER        layer = RENDER_LAYER::BLEND;
    uint32_t            flags = RF_NONE;

    _uint               iRow = 1;
    _uint               iCol = 1;
    _uint               iTotalFrame = 1;

    _float              fFrameDuration = 0.05f;

    _bool               bLoop = false;
    _bool               bPlay = true;
    _bool               bBillboard = true;
    _bool               bFinished = false;

    _float2             vSize = { 1.f, 1.f };
    _float4             vColor = { 1.f, 1.f, 1.f, 1.f };

    /* runtime */
    _float              fAccTime = 0.f;
    _uint               iCurFrame = 0;
} SPRITE_EFFECT_DATA;


class ENGINE_DLL CSpriteEffect final
: public CComponent_Proxy_Base<SPRITE_EFFECT_DATA, CSpriteEffect, COMPONENT_TYPE::SPRITE_EFFECT>
{
public:
    CSpriteEffect();
    CSpriteEffect(DataType* pData, COMPONENT_HANDLE hComponent);
    virtual ~CSpriteEffect() override = default;


public:
    void Set_Texture(uint32_t hTexture);
    uint32_t Get_Texture() const;

    void Set_Material(uint32_t hMaterial);
    void Set_FrameInfo(_uint iRow, _uint iCol, _uint iTotalFrame, _float fFrameDuration);
    void Set_Loop(_bool bLoop);
    void Set_Play(_bool bPlay);
    void Play();
    void Stop();
    void Reset();
    void Play_From_Start();
    void Set_Billboard(_bool bBillboard);
    void Set_Size(const _float2& vSize);
    void Set_Color(const _float4& vColor);
    _uint Get_CurrentFrame() const;
    _bool Is_Finished() const;
};


NS_END
