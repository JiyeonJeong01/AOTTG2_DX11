#include "Solver.h"

#include "GameObject.h"
#include "Transform.h"
#include "Rigidbody.h"
#include "Collider.h"
#include "Engine_Math.h"

NS_BEGIN(Engine)

CSolver::CSolver()
{
}

CSolver::~CSolver()
{
}

void CSolver::Solve_Contacts(CONTACT_DESC* pInfo)
{
    if (pInfo == nullptr)
        return;

    /* 수직 항력(Normal Impulse)과 마찰력(Friction Impulse) */
    Solve_Impulse(pInfo);

    /* 파고듦 해결 */
    Solve_Penetration(pInfo);
}

void CSolver::Solve_Impulse(CONTACT_DESC* pInfo)
{
    if (pInfo == nullptr || pInfo->pColA == nullptr || pInfo->pColB == nullptr)
        return;

    if (pInfo->fDepth <= 0.f)
        return;

    if (Is_Separating(pInfo)) /* 이미 멀어지는 중인 경우 중복 solve 하지 않는다. */
        return;

    CRigidbody rigidbodyA;
    CRigidbody rigidbodyB;
    CTransform transformA;
    CTransform transformB;

    RIGIDBODY_DATA* pBodyA = nullptr;
    RIGIDBODY_DATA* pBodyB = nullptr;

    Try_Get_Rigidbody_And_Transform(pInfo->pColA, &rigidbodyA, &transformA, &pBodyA);
    Try_Get_Rigidbody_And_Transform(pInfo->pColB, &rigidbodyB, &transformB, &pBodyB);

    /* 최소 한 쪽이 dynamic이어야 물리 solve 가능 */
    if ((pBodyA == nullptr || pBodyA->eBodyType != BODY_TYPE::DYNAMIC) &&
        (pBodyB == nullptr || pBodyB->eBodyType != BODY_TYPE::DYNAMIC))
        return;

    const _float3 vPoint = pInfo->vPoint;

    /* --------------------------------------------------------------------------------
     * TODO : 테스트 필요한 부분
     * TODO : 현재 vPenetrateN를 법선처럼 사용하고 있다. 이 값은 A가 B를 침투하는 방향이다.
     * TODO : 모든 충돌 케이스에서 올바른 contact normal은 아닐 수 있다.
       ------------------------------------------------------------------------------- */

    /* -------------------------- Solve Normal -------------------------- */
    const _float3 vResolveN_A = pInfo->vResolveN_A;
    _float3 vNorm; /* A가 B로 침범하는 방향 */
    Math::Store(vNorm, Math::Load(vResolveN_A) * -1.f);

    const _vector vPointVec = Math::Load(vPoint);
    const _vector vResolveN = Math::Load(vNorm);

    /* 충돌 지점에서의 속도 구하기 */
    const _float3 vVel_A = Calc_PointVelocity(pBodyA, vPoint);
    const _float3 vVel_B = Calc_PointVelocity(pBodyB, vPoint);
    const _vector vVelA = Math::Load(vVel_A);
    const _vector vVelB = Math::Load(vVel_B);

    /* B가 A에 대해 움직이는 상대 속도 */
    const _vector vVelRel = vVelB - vVelA;

    /* 충돌 지점에서 COM 으로의 벡터 벡터 r 구하기 */
    const _float3 vCOM_A = (pBodyA != nullptr) ? pBodyA->vCOM : Math::Zero3();
    const _float3 vCOM_B = (pBodyB != nullptr) ? pBodyB->vCOM : Math::Zero3();
    const _vector vCOMA = Math::Load(vCOM_A);
    const _vector vCOMB = Math::Load(vCOM_B);
    const _vector vR_A = vPointVec - vCOMA;
    const _vector vR_B = vPointVec - vCOMB;

    /* 이번 충돌로 인한 Normal Impulse가 만들어내는 회전 효과의 방향 구하기 */
    const _vector vJAxis_A = Math::Cross(vR_A, vResolveN);
    const _vector vJAxis_B = Math::Cross(vR_B, vResolveN);

    _float3 vJAxisA{};
    _float3 vJAxisB{};
    Math::Store(vJAxisA, vJAxis_A);
    Math::Store(vJAxisB, vJAxis_B);

    /* 해당 축으로 얼마나 잘 도는지 구하기 */
    const _float fInvInertia_A = Calc_InvInertiaOfAxis(transformA, pBodyA, vJAxisA);
    const _float fInvInertia_B = Calc_InvInertiaOfAxis(transformB, pBodyB, vJAxisB);

    const _float fVel_Norm = Math::Get_X(Math::Dot(vResolveN, vVelRel));

    /* 임의 fThreshold : 이 값보다 작은 반발은 무시하여 Jitter 방지하기 */
    const _float fThreshold = 9.81f / 5.1f;
    _float fRestitution = 0.f;
    {
        const _float fRestA = Get_Restitution(pBodyA);
        const _float fRestB = Get_Restitution(pBodyB);
        fRestitution = (fabsf(fVel_Norm) < fThreshold) ? 0.f : max(fRestA, fRestB);
    }

    /* 파고드는 속도를 제거하는 과정 */
    const _float fNumerator = -(fRestitution + 1.f) * fVel_Norm;

    const _float fInvMassA = Get_InvMass(pBodyA);
    const _float fInvMassB = Get_InvMass(pBodyB);

    const _float fMagSq_J_A =  Math::Get_X(Math::Dot(vJAxis_A, vJAxis_A));
    const _float fMagSq_J_B =  Math::Get_X(Math::Dot(vJAxis_B, vJAxis_B));

    const _float fDenominator =
        fInvMassA + fInvMassB +
        fMagSq_J_A * fInvInertia_A +
        fMagSq_J_B * fInvInertia_B;

    if (fDenominator <= 0.f)
        return;

    _float fJ = fNumerator / fDenominator;
    if (fJ < 0.f)
        fJ = 0.f;

    _float3 vImpulse{};
    Math::Store(vImpulse, vResolveN * fJ);

    _float3 tmpvResolveN;
    Math::Store(tmpvResolveN, vResolveN);

    if (!Math::Is_Zero(vResolveN * fJ))
    {
        Add_ImpulseAtPoint(transformA, pBodyA, _float3{ -vImpulse.x, -vImpulse.y, -vImpulse.z }, vPoint);
        Add_ImpulseAtPoint(transformB, pBodyB, vImpulse, vPoint);
    }

    /* -------------------------- Solve Friction -------------------------- */

    /* 접선 성분 구하기 */
    const _vector vVel_Parallel = vResolveN * fVel_Norm;
    const _vector vVel_TangentRaw = vVelRel - vVel_Parallel;

    /* 미끄러지는 방향 */
    _float3 vVelTangentRaw3{};
    Math::Store(vVelTangentRaw3, vVel_TangentRaw);

    _float3 vJFriction = Math::Zero3();

    if (!Math::Is_Zero(vVelTangentRaw3))
    {
        const _vector vVelTangent = Math::Normalize(vVel_TangentRaw);
        const _float fNumeratorT = Math::Get_X(Math::Dot(vVelRel, vVelTangent));

        const _vector vTangentAxis_A = Math::Cross(vR_A, vVelTangent);
        const _vector vTangentAxis_B = Math::Cross(vR_B, vVelTangent);

        _float3 vTangentAxisA{};
        _float3 vTangentAxisB{};
        Math::Store(vTangentAxisA, vTangentAxis_A);
        Math::Store(vTangentAxisB, vTangentAxis_B);

        const _float fInvInertiaT_A = Calc_InvInertiaOfAxis(transformA, pBodyA, vTangentAxisA);
        const _float fInvInertiaT_B = Calc_InvInertiaOfAxis(transformB, pBodyB, vTangentAxisB);

        const _float fMagSq_TAxis_A = Math::Get_X(Math::Dot(vTangentAxis_A, vTangentAxis_A));
        const _float fMagSq_TAxis_B = Math::Get_X(Math::Dot(vTangentAxis_B, vTangentAxis_B));

        const _float fDenominatorT =
            fInvMassA + fInvMassB +
            fMagSq_TAxis_A * fInvInertiaT_A +
            fMagSq_TAxis_B * fInvInertiaT_B;

        if (fDenominatorT > 0.f)
        {
            _float fImpulseT = -fNumeratorT / fDenominatorT;

            const _float fFrictionA = Get_Friction(pBodyA);
            const _float fFrictionB = Get_Friction(pBodyB);

            const _float mu = sqrtf(fFrictionA * fFrictionB);
            const _float fMaxFriction = fJ * mu;

            if (fImpulseT > fMaxFriction)
                fImpulseT = fMaxFriction;
            if (fImpulseT < -fMaxFriction)
                fImpulseT = -fMaxFriction;

            Math::Store(vJFriction, vVelTangent * fImpulseT);
        }
    }

    if (!Math::Is_NearlyZero(vJFriction))
    {
        Add_ImpulseAtPoint(transformA, pBodyA, _float3{ -vJFriction.x, -vJFriction.y, -vJFriction.z }, vPoint);
        Add_ImpulseAtPoint(transformB, pBodyB, vJFriction, vPoint);
    }
}

void CSolver::Solve_Penetration(CONTACT_DESC* pInfo)
{
    if (pInfo == nullptr || pInfo->pColA == nullptr || pInfo->pColB == nullptr)
        return;

    if (pInfo->fDepth <= 0.f)
        return;

    CRigidbody rigidbodyA, rigidbodyB;
    CTransform transformA, transformB;

    RIGIDBODY_DATA* pBodyA = nullptr;
    RIGIDBODY_DATA* pBodyB = nullptr;

    Try_Get_Rigidbody_And_Transform(pInfo->pColA, &rigidbodyA, &transformA, &pBodyA);
    Try_Get_Rigidbody_And_Transform(pInfo->pColB, &rigidbodyB, &transformB, &pBodyB);

    const _float fMoveWeightA = (pBodyA == nullptr) ? 0.f : 1.f;
    const _float fMoveWeightB = (pBodyB == nullptr) ? 0.f : 1.f;

    const _float fTotalWeight = fMoveWeightA + fMoveWeightB;
    if (fTotalWeight <= 0.f)
        return;

    const _float fMoveA = pInfo->fDepth * (fMoveWeightA / fTotalWeight);
    const _float fMoveB = pInfo->fDepth * (fMoveWeightB / fTotalWeight);

    /* TODO : vResolveN_A를 penetration normal처럼 사용하고 있다.
     * TODO : 현재는 A를 B에서 분리하는 방향을 규약으로 동작하지만,
     * TODO : 추후 NarrowPhase가 진짜 contact normal을 제공하면 그 값으로 통일하는 게 더 안전하다. */
    const _vector vResolveN = Math::Load(pInfo->vResolveN_A);

    if (fMoveWeightA > 0.f && transformA.Is_Valid())
    {
        transformA.Translate(vResolveN * fMoveA, SPACE::WORLD);

        if (pBodyA != nullptr)
        {
            TRANSFORM_DATA* pTrA = transformA._Data();
            if (pTrA != nullptr)
            {
                pBodyA->vCOM = pTrA->vPosition;
                pBodyA->vWorldCOM = pTrA->vPosition;
            }
        }
    }

    if (fMoveWeightB > 0.f && transformB.Is_Valid())
    {
        transformB.Translate(vResolveN * (-fMoveB), SPACE::WORLD);

        if (pBodyB != nullptr)
        {
            TRANSFORM_DATA* pTrB = transformB._Data();
            if (pTrB != nullptr)
            {
                pBodyB->vCOM = pTrB->vPosition;
                pBodyB->vWorldCOM = pTrB->vPosition;
            }
        }
    }
}

void CSolver::Add_ImpulseAtPoint(CTransform transform, RIGIDBODY_DATA* pBody, const _float3& vImpulse, const _float3& vPoint)
{
    if (pBody == nullptr)
        return;

    if (pBody->eBodyType != BODY_TYPE::DYNAMIC)
        return;

    const _vector vImpulseVec = Math::Load(vImpulse);

    /* Linear impulse */
    {
        const _vector vLinearVel = Math::Load(pBody->vLinearVel);
        Math::Store(pBody->vLinearVel, vLinearVel + vImpulseVec * pBody->fInvMass);
    }

    /* Angular impulse */
    {
        const _vector vPointVec = Math::Load(vPoint);
        const _vector vCOM = Math::Load(pBody->vCOM);
        const _vector vR = vPointVec - vCOM;

        _float3 vR3{};
        Math::Store(vR3, vR);
        if (Math::Is_NearlyZero(vR3))
            return;

        const _vector vAngularImpulse = Math::Cross(vR, vImpulseVec);

        /* 이미 Rigidbody_Builder에서 world inverse inertia를 계산해둔 것을 사용한다. */
        const _matrix matInvWorldInertia = Math::Load(pBody->matWorldInvInertiaTensor);
        const _vector vDeltaW = XMVector3TransformNormal(vAngularImpulse, matInvWorldInertia);

        const _vector vAngularVel = Math::Load(pBody->vAngularVel);
        Math::Store(pBody->vAngularVel, vAngularVel + vDeltaW);
    }
}

_bool CSolver::Is_Separating(CONTACT_DESC* pInfo)
{
    if (pInfo == nullptr)
        return false;

    if (pInfo->pColA == nullptr || pInfo->pColB == nullptr)
        return false;

    CRigidbody rigidbodyA;
    CRigidbody rigidbodyB;
    CTransform transformA;
    CTransform transformB;

    RIGIDBODY_DATA* pBodyA = nullptr;
    RIGIDBODY_DATA* pBodyB = nullptr;

    Try_Get_Rigidbody_And_Transform(pInfo->pColA, &rigidbodyA, &transformA, &pBodyA);
    Try_Get_Rigidbody_And_Transform(pInfo->pColB, &rigidbodyB, &transformB, &pBodyB);

    const _float3 vA = Calc_PointVelocity(pBodyA, pInfo->vPoint);
    const _float3 vB = Calc_PointVelocity(pBodyB, pInfo->vPoint);

    const _vector vRel = Math::Load(vB) - Math::Load(vA);

    /* --------------------------------------------------------------------------------
     * TODO : 테스트 필요한 부분
     * 현재 vResolveN_A를 법선처럼 사용하고 있다. 이 값은 A를 B에게서 밀어내는 방향이다.
     * 모든 충돌 케이스에서 수학적으로 contact normal은 아닐 수 있다.
       ------------------------------------------------------------------------------- */
    const _vector vResolveN = Math::Load(pInfo->vResolveN_A);

    const _float vn = Math::Get_X(Math::Dot(vRel, vResolveN));
    const _float eps = 1e-4f;

    _bool bSperating = vn < -eps;
    return bSperating;
}

_float3 CSolver::Calc_PointVelocity(RIGIDBODY_DATA* pBody, const _float3& vPoint)
{
    if (pBody == nullptr)
        return Math::Zero3();

    const _vector vPointVec = Math::Load(vPoint);
    const _vector vCOM = Math::Load(pBody->vCOM);
    const _vector vAngularVel = Math::Load(pBody->vAngularVel);
    const _vector vLinearVel = Math::Load(pBody->vLinearVel);

    const _vector vComToPoint = vPointVec - vCOM;
    const _vector vRot = Math::Cross(vAngularVel, vComToPoint);

    _float3 vOut{};
    Math::Store(vOut, vLinearVel + vRot);
    return vOut;
}

_float CSolver::Calc_InvInertiaOfAxis(CTransform transform, RIGIDBODY_DATA* pBody, const _float3& vAxis)
{
    if (pBody == nullptr)
        return 0.f;

    if (pBody->eBodyType != BODY_TYPE::DYNAMIC)
        return 0.f;

    if (Math::Is_NearlyZero(vAxis))
        return 0.f;

    const _vector vBaseAxis = Math::Normalize(Math::Load(vAxis));

    /* 이미 world inverse inertia를 가지고 있으므로 그대로 사용 */
    const _matrix matInvWorldInertia = Math::Load(pBody->matWorldInvInertiaTensor);
    const _vector vInvAxis = XMVector3TransformNormal(vBaseAxis, matInvWorldInertia);

    return Math::Get_X(Math::Dot(vBaseAxis, vInvAxis));
}

_bool CSolver::Try_Get_Rigidbody_And_Transform(
    COLLIDER_DATA* pColData,
    CRigidbody* pOutRigidbody,
    CTransform* pOutTransform,
    RIGIDBODY_DATA** ppOutBody)
{
    if (pOutRigidbody != nullptr)
        *pOutRigidbody = CRigidbody();

    if (pOutTransform != nullptr)
        *pOutTransform = CTransform();

    if (ppOutBody != nullptr)
        *ppOutBody = nullptr;

    if (pColData == nullptr)
        return false;

    CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pColData->hObject);
    if (pObj == nullptr)
        return false;

    CTransform transform = pObj->Get_Component<CTransform>();
    if (pOutTransform != nullptr)
        *pOutTransform = transform;

    CRigidbody rigidbody = pObj->Get_Component<CRigidbody>();
    if (pOutRigidbody != nullptr)
        *pOutRigidbody = rigidbody;

    if (ppOutBody != nullptr)
    {
        if (rigidbody.Is_Valid())
            *ppOutBody = rigidbody._Data();
        else
            *ppOutBody = nullptr;
    }

    return true;
}

_float CSolver::Get_InvMass(RIGIDBODY_DATA* pBody) const
{
    if (pBody == nullptr)
        return 0.f;

    if (pBody->eBodyType != BODY_TYPE::DYNAMIC)
        return 0.f;

    return pBody->fInvMass;
}

_float CSolver::Get_Restitution(RIGIDBODY_DATA* pBody) const
{
    if (pBody == nullptr)
        return 0.f;

    return pBody->fRestitution;
}

_float CSolver::Get_Friction(RIGIDBODY_DATA* pBody) const
{
    if (pBody == nullptr)
        return 1.f;

    return pBody->fFriction;
}

std::unique_ptr<CSolver> CSolver::Create()
{
    return std::make_unique<CSolver>();
}

NS_END
