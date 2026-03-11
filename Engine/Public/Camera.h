// Camera.h
#pragma once
#include "Engine_Define.h"
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagCameraData
{
    OBJECT_HANDLE       hObject{};
    _bool               bEnable = false;

    COMPONENT_HANDLE    hTransform{};

    /* projection */
    _bool    bOrthographic = false;
    _float   fFovy = 60.f;          /* degrees (perspective) */
    _float   fOrthoSize = 5.f;      /* half-height (orthographic) */
    _float   fAspect = 16.f / 9.f;
    _float   fNear = 0.1f;
    _float   fFar = 1000.f;

    /* output / render control */
    uint32_t layerMask = 0xFFFFFFFFu;
    uint8_t  iPriority = 1;

    /* cache flags */
    _bool    dirty = true;
} CAMERA_DATA;

class ENGINE_DLL CCamera final : public CComponent_Proxy_Base<CAMERA_DATA, CCamera, COMPONENT_TYPE::CAMERA>
{
public:
    CCamera() : CComponent_Proxy_Base() {}
    CCamera(DataType* pData, COMPONENT_HANDLE handle) : CComponent_Proxy_Base(pData, handle) {}
    ~CCamera() override = default;
public:
    void Set_Priority(uint8_t iPriority);
    void Set_Ortho(_bool bOrthographic);
    void Set_Fovy(_float fFovy);
    void Set_OrthoSize(_float fOrthoSize);
    void Set_Aspect(_float fAspect);
    void Set_NearFar(_float fNear, _float fFar);
    void Set_LayerMask(uint32_t uintLayerMask);
};

NS_END
