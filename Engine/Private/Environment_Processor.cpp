// Environment_System.cpp
#include "Environment_Processor.h"

#include "Component_System.h"
#include "CRender_System.h"

#include "Engine_Log.h"
#include "Component_Spec.h"
#include "Transform_Processor.h"

NS_BEGIN(Engine)


CEnvironment_Processor::CEnvironment_Processor() = default;
CEnvironment_Processor::~CEnvironment_Processor() = default;

std::unique_ptr<CEnvironment_Processor> CEnvironment_Processor::Create()
{
    auto pInstance = std::make_unique<CEnvironment_Processor>();
    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create Instance failed");

    return pInstance;
}

_matrix CEnvironment_Processor::Calculate_ViewMatrix(TRANSFORM_DATA* pCameraTr)
{
    _vector vEye = Math::Load(pCameraTr->vPosition);

    _vector vForward;
    memcpy(&vForward, &pCameraTr->matWorld.m[2][0], sizeof(_float4));
    vForward = XMVector3Normalize(vForward);

    _vector vUp;
    memcpy(&vUp, &pCameraTr->matWorld.m[1][0], sizeof(_float4));
    vUp = XMVector3Normalize(vUp);

    _vector vAt = vEye + vForward;

    _matrix matView = XMMatrixLookAtLH(vEye, vAt, vUp);
    return matView;
}

_matrix CEnvironment_Processor::Calculate_ProjMatrix(CAMERA_DATA* pData)
{
    if (!pData)
        return XMMatrixIdentity();

    const _float fAspect = (pData->fAspect > 0.001f) ? pData->fAspect : 0.001f;
    const _float fNear = (pData->fNear > 0.001f) ? pData->fNear : 0.001f;
    const _float fFar = (pData->fFar > fNear) ? pData->fFar : (fNear + 0.001f);

    if (pData->bOrthographic)
    {
        const _float fOrthoSize = (pData->fOrthoSize > 0.001f) ? pData->fOrthoSize : 0.001f;
        const _float fWidth = fOrthoSize * 2.f * fAspect;
        const _float fHeight = fOrthoSize * 2.f;

        return XMMatrixOrthographicLH(fWidth, fHeight, fNear, fFar);
    }

    _float fFovy = pData->fFovy;
    if (fFovy < 1.f)
        fFovy = 1.f;
    if (fFovy > 179.f)
        fFovy = 179.f;

    return XMMatrixPerspectiveFovLH(XMConvertToRadians(fFovy), fAspect, fNear, fFar);
}

HRESULT CEnvironment_Processor::Initialize()
{
    /* --- Register Factory --- */
    {
        SYS_COMPONENT.Register_InitialSpecFactory<CCamera, CAMERA_SPEC>();
        SYS_COMPONENT.Register_BuildSpecFacotry<CCamera>();

        //SYS_COMPONENT.Register_InitialSpecFactory<CLight, LIGHT_SPEC>();
        //SYS_COMPONENT.Register_BuildSpecFacotry<CLight>();
    }

    m_pTransformProcessor = SYS_COMPONENT.Bind_Processor<CTransform_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pTransformProcessor, E_FAIL, "transform processor is nullptr");

    return S_OK;
}

void CEnvironment_Processor::Update(_float)
{
    CAMERA_DATA* pPriorityCamera = nullptr;
    TRANSFORM_DATA* pCameraTr = nullptr;
    uint8_t iPriority = 0;

    const auto& pages = m_CameraPool.GetPages();
    for (const auto& upPage : pages)
    {
        auto* pPage = upPage.get();
        if (!pPage)
            continue;
        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            CAMERA_DATA* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable)
                continue;

            TRANSFORM_DATA* pTrData = m_pTransformProcessor->Get_Proxy(COMPONENT_TYPE::TRANSFORM, pData->hTransform)._Data();
            if (!pTrData)
                continue;

            if (pPriorityCamera == nullptr || iPriority < pData->iPriority) /* 최소 하나 보장, 우선순위가 큰 카메라 선택 */
            {
                iPriority = pData->iPriority;
                pPriorityCamera = pData;
                pCameraTr = pTrData;
            }
        }
    }

    if (pPriorityCamera && pCameraTr)
    {
        /* view 설정 */

        _matrix matView  = Calculate_ViewMatrix(pCameraTr);
        _matrix matProj = Calculate_ProjMatrix(pPriorityCamera);

        SYS_RENDER.Submit_Camera(matView, matProj);
    }

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

void* CEnvironment_Processor::Get_DataPtr(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) noexcept
{
    switch (eComType)
    {
    case COMPONENT_TYPE::CAMERA:  return m_CameraPool.Get_Data_By_Handle(hComponent);
    case COMPONENT_TYPE::LIGHT: return m_LightPool.Get_Data_By_Handle(hComponent);
    default: return nullptr;
    }
}

HRESULT CEnvironment_Processor::Initialize_Component_Data(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::CAMERA :
        {
            CAMERA_DATA* pData = m_CameraPool.Get_Data_By_Handle(hComponent);
            IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "pData is nullptr");

            CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
            IF_NULL_RETURN_MSG_BREAK(pObj, E_FAIL, "pObj is nullptr");

            CTransform tr = pObj->Get_Component<CTransform>();
            IF_TRUE_RETURN_MSG_BREAK(!tr.Is_Valid(), E_FAIL, "tr is not valid");

            pData->hTransform = tr.Get_Handle();

            return S_OK;
        }
    case COMPONENT_TYPE::LIGHT:
        return S_OK;
    }

    return E_FAIL;
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
    d->iPriority = p->bEnabled;
    d->dirty = true;

    return S_OK;
}
HRESULT CEnvironment_Processor::Initialize_From_Spec_Light(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec)
{
    const auto* p = SCAST(const LIGHT_SPEC*, spec);
    IF_NULL_RETURN_MSG_BREAK(p, E_FAIL, "LightSpec cast failed.");

    auto* d = m_LightPool.Get_Data_By_Handle(h);
    IF_NULL_RETURN_MSG_BREAK(d, E_FAIL, "Light handle invalid.");

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
    up->bEnabled = d->iPriority;

    return up;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CEnvironment_Processor::Build_Spec_Light(COMPONENT_HANDLE h)
{
    auto* d = m_LightPool.Get_Data_By_Handle(h);
    IF_NULL_RETURN_MSG_BREAK(d, nullptr, "Light handle invalid.");

    auto up = std::make_unique<LIGHT_SPEC>();

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
