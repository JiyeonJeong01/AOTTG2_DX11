#pragma once
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

typedef struct tagSpringJointData final
{
    OBJECT_HANDLE    hObject{};
    _bool            bEnable = true;
    _bool            bUseSpring = false;
    uint8_t          pad0[2] = {};

    COMPONENT_HANDLE hRigidbody = INVALID_HANDLE;

    _float3          vAnchor{ 0.f, 0.f, 0.f };

    _float           fSpring = 0.f;
    _float           fDamper = 0.f;
    _float           fRestLength = 0.f;

    _float           fMinLength = 0.f;
    _float           fMaxLength = 0.f;

    _bool            bUseMinLength = false;
    _bool            bUseMaxLength = false;
    uint8_t          pad1[2] = {};

} SPRING_JOINT_DATA;

class ENGINE_DLL CSpringJoint final : public CComponent_Proxy_Base<SPRING_JOINT_DATA, CSpringJoint, COMPONENT_TYPE::SPRING_JOINT>
{
public:
    CSpringJoint() : CComponent_Proxy_Base()  {  }
    CSpringJoint(DataType* pData, COMPONENT_HANDLE handle) : CComponent_Proxy_Base(pData, handle)  { }
    ~CSpringJoint() override = default;

public:
    void        Set_UseSpring(_bool bUseSpring);
    _bool       Get_UseSpring() const;

    void        Set_Anchor(const _float3& vAnchor);
    _float3     Get_Anchor() const;

    void        Set_Spring(_float fSpring);
    _float      Get_Spring() const;

    void        Set_Damper(_float fDamper);
    _float      Get_Damper() const;

    void        Set_RestLength(_float fRestLength);
    _float      Get_RestLength() const;

    void        Set_MinLength(_float fMinLength);
    _float      Get_MinLength() const;

    void        Set_MaxLength(_float fMaxLength);
    _float      Get_MaxLength() const;

    void        Set_UseMinLength(_bool bUseMinLength);
    _bool       Get_UseMinLength() const;

    void        Set_UseMaxLength(_bool bUseMaxLength);
    _bool       Get_UseMaxLength() const;
};

NS_END
