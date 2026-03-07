#pragma once

#include "Physics_Struct.h"
#include "Rigidbody.h"
#include "Transform.h"

NS_BEGIN(Engine)
    class CGameObject;
class CCollider;
class CTransform;

class ENGINE_DLL CRigidbody_Builder final
{
public:
    CRigidbody_Builder();
    ~CRigidbody_Builder();

public:
    HRESULT Rebuild(RIGIDBODY_DATA* pData);

private:
    void    Calc_Dimension(RIGIDBODY_DATA* pData, COLLIDER_DATA* pColData);
    void    Calc_InvMass(RIGIDBODY_DATA* pData);
    void    Calc_InertiaTensor(RIGIDBODY_DATA* pData);
    void    Calc_WorldInertiaTensor(RIGIDBODY_DATA* pData, TRANSFORM_DATA* pTrData);
    void    Calc_COM(RIGIDBODY_DATA* pData, TRANSFORM_DATA* pTrData);
    HRESULT Validate(RIGIDBODY_DATA* pData, _Out_  CGameObject** ppObj, _Out_ TRANSFORM_DATA** ppTrData, _Out_ COLLIDER_DATA** ppColData);
public :
    static std::unique_ptr<CRigidbody_Builder> Create();
};

NS_END
