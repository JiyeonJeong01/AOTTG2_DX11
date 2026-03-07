#pragma once

#include "Rigidbody.h"
#include "Physics_Struct.h"

NS_BEGIN(Engine)

class CGameObject;
class CTransform;
class CRigidbody;

class ENGINE_DLL CSolver final
{
public:
    CSolver();
    ~CSolver();

public:
    void    Solve_Contacts(CONTACT_DESC* pInfo);

private:
    void    Solve_Impulse(CONTACT_DESC* pInfo);
    void    Solve_Penetration(CONTACT_DESC* pInfo);

    void    Add_ImpulseAtPoint(CTransform transform, RIGIDBODY_DATA* pBody, const _float3& vImpulse, const _float3& vPoint);

    _bool   Is_Separating(CONTACT_DESC* pInfo);
    _float3 Calc_PointVelocity(RIGIDBODY_DATA* pBody, const _float3& vPoint);
    _float  Calc_InvInertiaOfAxis(CTransform transform, RIGIDBODY_DATA* pBody, const _float3& vAxis);

private:
    _bool   Try_Get_Rigidbody_And_Transform(
        COLLIDER_DATA* pColData,
        CRigidbody* pOutRigidbody,
        CTransform* pOutTransform,
        RIGIDBODY_DATA** ppOutBody);

    _float  Get_InvMass(RIGIDBODY_DATA* pBody) const;
    _float  Get_Restitution(RIGIDBODY_DATA* pBody) const;
    _float  Get_Friction(RIGIDBODY_DATA* pBody) const;

public :
    static std::unique_ptr<CSolver> Create();
};

NS_END
