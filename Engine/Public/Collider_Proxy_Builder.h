#pragma once

#include "Engine_Define.h"
#include "Physics_Struct.h"
#include "Collider.h"
#include "Transform.h"

NS_BEGIN(Engine)

class CTransform_Processor;

class CCollider_Proxy_Builder final
{
public :
    CCollider_Proxy_Builder();
    ~CCollider_Proxy_Builder();
public:
    HRESULT Initialize();
    void Build_Collider_Proxy(COLLIDER_DATA* pData, COLLIDER_PROXY_DATA& outProxy);

private :
    CTransform_Processor*    m_pTransform_Processor;

private :
    void Build_Box_Proxy(COLLIDER_DATA* pCol, TRANSFORM_DATA* pTr, COLLIDER_PROXY_DATA& outProxy);
    void Build_Sphere_Proxy(COLLIDER_DATA* pCol, TRANSFORM_DATA* pTr, COLLIDER_PROXY_DATA& outProxy);
    void Build_Plane_Proxy(COLLIDER_DATA* pCol, TRANSFORM_DATA* pTr, COLLIDER_PROXY_DATA& outProxy);

    inline _vector Get_ColliderCenter_World(COLLIDER_DATA* pCol, TRANSFORM_DATA* pTr);
    inline _float Get_SphereRadius_World(COLLIDER_DATA* pCol, TRANSFORM_DATA* pTr);

public :
    static std::unique_ptr<CCollider_Proxy_Builder> Create();
};


NS_END
