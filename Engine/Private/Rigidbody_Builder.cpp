#include "Rigidbody_Builder.h"

#include "GameObject.h"
#include "Transform.h"
#include "Collider.h"

NS_BEGIN(Engine)

namespace
{
    inline _float4x4 Make_Identity_Matrix()
    {
        return Math::Identity();
    }

    inline void Set_Identity_Matrix(_float4x4& mat)
    {
        mat = Math::Identity();
    }

    inline _bool Is_Dynamic_Body(const RIGIDBODY_DATA* pData)
    {
        return (pData->eBodyType == BODY_TYPE::DYNAMIC);
    }
}

CRigidbody_Builder::CRigidbody_Builder()
{

}

CRigidbody_Builder::~CRigidbody_Builder()
{

}

HRESULT CRigidbody_Builder::Rebuild(RIGIDBODY_DATA* pData)
{
    CGameObject* pObj = nullptr;
    TRANSFORM_DATA* pTrData = nullptr;
    COLLIDER_DATA* pColData = nullptr;

    IF_FAIL_RETURN_MSG_BREAK(Validate(pData, &pObj, &pTrData, &pColData), E_FAIL, "pData failed to validate.");

    Calc_Dimension(pData, pColData);
    Calc_InvMass(pData);
    Calc_InertiaTensor(pData);
    Calc_COM(pData, pTrData);
    Calc_WorldInertiaTensor(pData, pTrData);

    pData->bDirtyMass = false;
    pData->bDirtyInertia = false;
    pData->bDirtyWorldInertia = false;

    return S_OK;
}

HRESULT CRigidbody_Builder::Validate(RIGIDBODY_DATA* pData, _Out_ CGameObject** ppObj, _Out_  TRANSFORM_DATA** ppTrData, _Out_  COLLIDER_DATA** ppColData)
{
    /* 에러 확인 용도 */
    IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "pData is nullptr");

    CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
    IF_NULL_RETURN_MSG_BREAK(pObj, E_FAIL, "pObj is nullptr.");

    CTransform transform = pObj->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(!transform.Is_Valid(), E_FAIL, "transform is not valid");

    TRANSFORM_DATA* pTr = transform._Data();
    IF_NULL_RETURN_MSG_BREAK(pTr, E_FAIL, "pTr is nullptr.");

    CCollider collider = pObj->Get_Component<CCollider>();
    IF_TRUE_RETURN_MSG_BREAK(!collider.Is_Valid(), E_FAIL, "collider is not valid");

    COLLIDER_DATA* pColData = collider._Data();
    IF_NULL_RETURN_MSG_BREAK(pColData, E_FAIL, "pCollider is nullptr");

    *ppObj = pObj;
    *ppTrData = pTr;
    *ppColData = pColData;

    return S_OK;
}

void CRigidbody_Builder::Calc_Dimension(RIGIDBODY_DATA* pData, COLLIDER_DATA* pColData)
{
    if (pData->eShape == SHAPE::END)
        pData->eShape = pColData->eShape;

    switch (pColData->eShape)
    {
    case SHAPE::BOX:
    {
        const _float3& vHalfExtentsLocal = pColData->box.vHalfExtentsLocal;

        pData->vDimension.x = vHalfExtentsLocal.x * 2.f;
        pData->vDimension.y = vHalfExtentsLocal.y * 2.f;
        pData->vDimension.z = vHalfExtentsLocal.z * 2.f;

        pData->vDimensionCenter = pColData->vOffset;
        break;
    }

    case SHAPE::SPHERE:
    {
        const _float fRadiusLocal = pColData->sphere.fRadiusLocal;
        const _float fDiameter = fRadiusLocal * 2.f;

        pData->vDimension = _float3{ fDiameter, fDiameter, fDiameter };
        pData->vDimensionCenter = pColData->vOffset;
        break;
    }

    case SHAPE::PLANE:
    default:
    {
        pData->vDimension = _float3{ 0.f, 0.f, 0.f };
        pData->vDimensionCenter = pColData->vOffset;
        break;
    }
    }
}

void CRigidbody_Builder::Calc_InvMass(RIGIDBODY_DATA* pData)
{
    if (!Is_Dynamic_Body(pData))
    {
        pData->fInvMass = 0.f;
        return;
    }

    if (pData->fMass <= 0.f)
    {
        pData->fInvMass = 0.f;
        return;
    }

    pData->fInvMass = 1.f / pData->fMass;
}

void CRigidbody_Builder::Calc_InertiaTensor(RIGIDBODY_DATA* pData)
{
    Set_Identity_Matrix(pData->matInertiaTensor);
    Set_Identity_Matrix(pData->matInvInertiaTensor);

    if (!Is_Dynamic_Body(pData))
    {
        memset(&pData->matInertiaTensor.m[0][0], 0, sizeof(pData->matInertiaTensor));
        memset(&pData->matInvInertiaTensor.m[0][0], 0, sizeof(pData->matInvInertiaTensor));
        return;
    }

    if (pData->fMass <= 0.f)
    {
        memset(&pData->matInertiaTensor.m[0][0], 0, sizeof(pData->matInertiaTensor));
        memset(&pData->matInvInertiaTensor.m[0][0], 0, sizeof(pData->matInvInertiaTensor));
        return;
    }

    switch (pData->eShape)
    {
    case SHAPE::SPHERE:
    {
        const _float fRadius = pData->vDimension.x * 0.5f;
        const _float fInertia = (2.f / 5.f) * pData->fMass * fRadius * fRadius;

        pData->matInertiaTensor._11 = fInertia;
        pData->matInertiaTensor._22 = fInertia;
        pData->matInertiaTensor._33 = fInertia;

        if (fInertia > 0.f)
        {
            const _float fInvInertia = 1.f / fInertia;
            pData->matInvInertiaTensor._11 = fInvInertia;
            pData->matInvInertiaTensor._22 = fInvInertia;
            pData->matInvInertiaTensor._33 = fInvInertia;
        }
        break;
    }

    case SHAPE::BOX:
    {
        const _float3& vDim = pData->vDimension;
        const _float k = (1.f / 12.f) * pData->fMass;

        const _float fIx = k * (vDim.y * vDim.y + vDim.z * vDim.z);
        const _float fIy = k * (vDim.x * vDim.x + vDim.z * vDim.z);
        const _float fIz = k * (vDim.x * vDim.x + vDim.y * vDim.y);

        pData->matInertiaTensor._11 = fIx;
        pData->matInertiaTensor._22 = fIy;
        pData->matInertiaTensor._33 = fIz;

        pData->matInvInertiaTensor._11 = (fIx > 0.f) ? (1.f / fIx) : 0.f;
        pData->matInvInertiaTensor._22 = (fIy > 0.f) ? (1.f / fIy) : 0.f;
        pData->matInvInertiaTensor._33 = (fIz > 0.f) ? (1.f / fIz) : 0.f;
        break;
    }

    default:
        break;
    }
}

void CRigidbody_Builder::Calc_WorldInertiaTensor(RIGIDBODY_DATA* pData, TRANSFORM_DATA* pTrData)
{
    Set_Identity_Matrix(pData->matWorldInertiaTensor);
    Set_Identity_Matrix(pData->matWorldInvInertiaTensor);

    if (!Is_Dynamic_Body(pData))
        return;

    const _matrix matRot = Math::RotationQuaternionM(pTrData->vRotationQuat);
    const _matrix matRotT = Math::Transpose(matRot);

    /* DX row-vector 규약 기준 */
    Math::Store(pData->matWorldInertiaTensor, matRotT * Math::Load(pData->matInertiaTensor) * matRot);
    Math::Store(pData->matWorldInvInertiaTensor, matRotT * Math::Load(pData->matInvInertiaTensor) * matRot);
}

void CRigidbody_Builder::Calc_COM(RIGIDBODY_DATA* pData, TRANSFORM_DATA* pTrData)
{
    /* 현재 규약: COM = Position */
    pData->vCOM = pTrData->vPosition;
    pData->vWorldCOM = pTrData->vPosition;
}

std::unique_ptr<CRigidbody_Builder> CRigidbody_Builder::Create()
{
    return std::make_unique<CRigidbody_Builder>();
}


NS_END
