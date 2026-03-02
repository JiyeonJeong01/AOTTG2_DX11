// Environment_System.cpp
#include "Environment_Processor.h"
#include "Engine_Log.h"
#include "Component_Spec.h"

NS_BEGIN(Engine)

CEnvironment_Processor::CEnvironment_Processor() = default;

CEnvironment_Processor::~CEnvironment_Processor() = default;

std::unique_ptr<CEnvironment_Processor> CEnvironment_Processor::Create()
{
    return std::make_unique<CEnvironment_Processor>();
}

HRESULT CEnvironment_Processor::Initialize()
{
    return S_OK;
}

void CEnvironment_Processor::Update(_float)
{
    /* 필요시 카메라, 빛 update 로직  넣기 */
}

void CEnvironment_Processor::LateUpdate(_float)
{
    /* 필요 시 dirty 기반 캐시 업데이트 */
}

COMPONENT_HANDLE CEnvironment_Processor::Create_Component_Data(COMPONENT_TYPE eComType, OBJECT_HANDLE hObject)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::CAMERA:
        return Create_Component_Data_Inner<CCamera>(m_CameraPool, hObject);

    case COMPONENT_TYPE::LIGHT:
        return Create_Component_Data_Inner<CLight>(m_LightPool, hObject);

    default:
        break;
    }

    IF_TRUE_RETURN_MSG_BREAK(true, COMPONENT_HANDLE{}, "CEnvironment_Processor does not support this component type.");
}

void CEnvironment_Processor::Remove_Component(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::CAMERA:
        Remove_Component_Inner<CCamera>(m_CameraPool, hComponent);
        return;

    case COMPONENT_TYPE::LIGHT:
        Remove_Component_Inner<CLight>(m_LightPool, hComponent);
        return;

    default:
        break;
    }

    _DEBUG_WARN("CEnvironment_Processor::Remove_Component - unsupported component type");
}

HRESULT CEnvironment_Processor::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec)
{
    IF_NULL_RETURN_MSG_BREAK(pSpec, E_FAIL, "Spec is nullptr.");

    switch (eComType)
    {
    case COMPONENT_TYPE::CAMERA:
        return Initialize_From_Spec_Camera(hComponent, pSpec);

    case COMPONENT_TYPE::LIGHT:
        return Initialize_From_Spec_Light(hComponent, pSpec);

    default:
        break;
    }

    IF_TRUE_RETURN_MSG_BREAK(true, E_FAIL, "Unsupported component type in Initialize_From_Spec.");
}

std::unique_ptr<COMPONENT_SPEC_BASE> CEnvironment_Processor::Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::CAMERA:
        return Build_Spec_Camera(hComponent);

    case COMPONENT_TYPE::LIGHT:
        return Build_Spec_Light(hComponent);

    default:
        break;
    }

    _DEBUG_WARN("CEnvironment_Processor::Build_Spec - unsupported component type");
    return nullptr;
}

void CEnvironment_Processor::Set_Enable(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, _bool bEnable)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::CAMERA:
        Set_Enable_Inner<CCamera>(m_CameraPool, hComponent, bEnable);
        return;

    case COMPONENT_TYPE::LIGHT:
        Set_Enable_Inner<CLight>(m_LightPool, hComponent, bEnable);
        return;

    default:
        break;
    }

    _DEBUG_WARN("CEnvironment_Processor::Remove_Component - unsupported component type");
}

HRESULT CEnvironment_Processor::Initialize_From_Spec_Camera(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec)
{
    const auto* p = SCAST(const CAMERA_SPEC*, spec);
    IF_NULL_RETURN_MSG_BREAK(p, E_FAIL, "CameraSpec cast failed.");

    auto* d = m_CameraPool.Get_Data_By_Handle(h);
    IF_NULL_RETURN_MSG_BREAK(d, E_FAIL, "Camera handle invalid.");

    d->bOrthographic = p->bOrthographic;
    d->fFovy = p->fovy;
    d->fOrthoSize = p->orthoSize;
    d->fAspect = p->aspect;
    d->fNear = p->zNear;
    d->fFar = p->zFar;
    d->layerMask = p->layerMask;
    d->bEnabled = p->bEnabled;
    d->dirty = true;

    return S_OK;
}
HRESULT CEnvironment_Processor::Initialize_From_Spec_Light(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec)
{
    const auto* p = SCAST(const LIGHT_SPEC*, spec);
    IF_NULL_RETURN_MSG_BREAK(p, E_FAIL, "LightSpec cast failed.");

    auto* d = m_LightPool.Get_Data_By_Handle(h);
    IF_NULL_RETURN_MSG_BREAK(d, E_FAIL, "Light handle invalid.");

    d->hObject = p->hObject;

    d->type = p->type;
    d->vColor = p->vColor;
    d->vDirection = p->vDirection;
    d->vPosition = p->vPosition;
    d->fRange = p->fRange;
    d->spotAngle = p->spotAngle;

    d->vDiffuse = p->vDiffuse;
    d->vAmbient = p->vAmbient;
    d->vSpecular = p->vSpecular;

    d->bEnabled = p->bEnabled;
    d->dirty = true;

    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CEnvironment_Processor::Build_Spec_Camera(COMPONENT_HANDLE h)
{
    auto* d = m_CameraPool.Get_Data_By_Handle(h);
    IF_NULL_RETURN_MSG_BREAK(d, nullptr, "Camera handle invalid.");

    auto up = std::make_unique<CAMERA_SPEC>();

    up->bOrthographic = d->bOrthographic;
    up->fovy = d->fFovy;
    up->orthoSize = d->fOrthoSize;
    up->aspect = d->fAspect;
    up->zNear = d->fNear;
    up->zFar = d->fFar;

    up->layerMask = d->layerMask;
    up->bEnabled = d->bEnabled;

    return up;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CEnvironment_Processor::Build_Spec_Light(COMPONENT_HANDLE h)
{
    auto* d = m_LightPool.Get_Data_By_Handle(h);
    IF_NULL_RETURN_MSG_BREAK(d, nullptr, "Light handle invalid.");

    auto up = std::make_unique<LIGHT_SPEC>();

    up->hObject = d->hObject;

    up->type = d->type;
    up->vColor = d->vColor;
    up->vDirection = d->vDirection;
    up->vPosition = d->vPosition;

    up->fRange = d->fRange;
    up->spotAngle = d->spotAngle;

    up->vDiffuse = d->vDiffuse;
    up->vAmbient = d->vAmbient;
    up->vSpecular = d->vSpecular;

    up->bEnabled = d->bEnabled;

    return up;
}

NS_END
