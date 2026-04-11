#pragma once
#include "CComponent_Proxy_Base.h"
#include "Physics_Struct.h"
#include "Transform.h"

NS_BEGIN(Engine)

typedef struct tagRigidbodyData final
{
    OBJECT_HANDLE   hObject{};
    _bool           bEnable = true;

    _bool           bDirtyMass = true;
    _bool           bDirtyInertia = true;
    _bool           bDirtyWorldInertia = true;

    _bool           bInitialized = false;
    _bool           bGravity = true;
    uint8_t         pad0[2] = {};

    COMPONENT_HANDLE hTransform = INVALID_HANDLE;
    COMPONENT_HANDLE hCollider = INVALID_HANDLE;

    SHAPE           eShape = SHAPE::END;
    BODY_TYPE       eBodyType = BODY_TYPE::DYNAMIC;

    _float          fMass = 1.f;
    _float          fInvMass = 0.f;

    _float          fDrag = 0.f;
    _float          fAngularDrag = 0.f;

    _float          fRestitution = 0.f;
    _float          fFriction = 1.f;

    _float3         vDimension = { 0.f, 0.f, 0.f };
    _float3         vDimensionCenter = { 0.f, 0.f, 0.f };

    _float3         vCOM = { 0.f, 0.f, 0.f }; /* 현재 기준 무조건 vPosition으로 통일한다. */
    _float3         vWorldCOM = { 0.f, 0.f, 0.f };

    _float3         vLinearVel = { 0.f, 0.f, 0.f };
    _float3         vAngularVel = { 0.f, 0.f, 0.f };

    _float3         vForceAccum = { 0.f, 0.f, 0.f };
    _float3         vTorqueAccum = { 0.f, 0.f, 0.f };

    _float4x4       matInertiaTensor = Math::Identity();
    _float4x4       matInvInertiaTensor = Math::Identity();
    _float4x4       matWorldInertiaTensor = Math::Identity();
    _float4x4       matWorldInvInertiaTensor = Math::Identity();

    AXIS_MASK       tRotationLock{};
    AXIS_MASK       tPositionLock{};

} RIGIDBODY_DATA;

class ENGINE_DLL CRigidbody final
    : public CComponent_Proxy_Base<RIGIDBODY_DATA, CRigidbody, COMPONENT_TYPE::RIGIDBODY>
{
public:
    CRigidbody()
        : CComponent_Proxy_Base()
    {
    }

    CRigidbody(DataType* pData, COMPONENT_HANDLE handle)
        : CComponent_Proxy_Base(pData, handle)
    {
    }

    ~CRigidbody() override = default;

public:
    void        Set_Shape(SHAPE eShape);
    SHAPE       Get_Shape() const;

    void        Set_BodyType(BODY_TYPE eBodyType);
    BODY_TYPE   Get_BodyType() const;

    void        Set_Mass(_float fMass);
    _float      Get_Mass() const;

    void        Set_Drag(_float fDrag);
    _float      Get_Drag() const;

    void        Set_AngularDrag(_float fAngularDrag);
    _float      Get_AngularDrag() const;

    void        Set_Restitution(_float fRestitution);
    _float      Get_Restitution() const;

    void        Set_Friction(_float fFriction);
    _float      Get_Friction() const;

    void        Set_Gravity(_bool bGravity);
    _bool       Get_Gravity() const;

    void        Set_COM(const _float3& vCOM);
    _float3     Get_COM() const;

    void        Set_LinearVel(const _float3& vLinearVel);
    _float3     Get_LinearVel() const;

    void        Set_AngularVel(const _float3& vAngularVel);
    _float3     Get_AngularVel() const;

    void        Set_RotationLock(const AXIS_MASK& tRotationLock);
    AXIS_MASK   Get_RotationLock() const;

    void        Set_PositionLock(const AXIS_MASK& tPositionLock);
    AXIS_MASK   Get_PositionLock() const;

    void        Translate(const _float3& vDeltaPos);

    void        Add_LinearImpulse(const _float3& vImpulse);
    void        Add_Force(const _float3& vForce);
    void        Add_Torque(const _float3& vTorque);

    void        Refresh();

private :
    TRANSFORM_DATA* m_pTrData{};

private :
    TRANSFORM_DATA* Find_Transform();
    
};

NS_END
