#pragma once
#include "Component_Processor.h"
#include "Component_Pool.h"

#include "Collider.h"
#include "Rigidbody.h"
#include "SpringJoint.h"

NS_BEGIN(Engine)
class CTransform_Processor;
class CCollision_Detector;
class CCollider_Proxy_Builder;
class CRigidbody_Builder;
class CSolver;
class CDebug_Renderer;

typedef struct tagRay RAY;
typedef struct tagRaycastHits RAYCAST_HITS;

class CPhysics_Processor :  public CComponent_Processor
{
    DEF_PROCESSOR_ID(PROCESSOR_ID::PHYSICS)
public:
    HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    void    LateUpdate(_float fDT) override;
    void    Fixed_Update(_float fDT);
    void    Render();

    COMPONENT_HANDLE    Create_Component_Data(COMPONENT_TYPE eComType, OBJECT_HANDLE hObject) override;
    void                Remove_Component(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;
    HRESULT             Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec) override;
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;
    void                Set_Enable(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, _bool bEnable) override;
    void*               Get_DataPtr(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) noexcept override;

    template<typename TProxy>
    TProxy Get_Proxy(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
    {
        if constexpr (std::is_same_v<TProxy, CCollider>) {
            return m_ColliderPool.Get_Proxy(hComponent);
        }
        if constexpr (std::is_same_v<TProxy, CRigidbody>) {
            return m_RigidbodyPool.Get_Proxy(hComponent);
        }
        if constexpr (std::is_same_v<TProxy, CSpringJoint>) {
            return m_SpringJointPool.Get_Proxy(hComponent);
        }

        IF_TRUE_RETURN_MSG_BREAK(true, TProxy{}, "Invalid Proxy Type for this Pool");
    }

private :
    template <typename TProxy>
    COMPONENT_HANDLE Create_Component_Data_Inner(CComponent_Pool<TProxy>& pool, OBJECT_HANDLE hObject)
    {
        COMPONENT_HANDLE hComponent = pool.Allocate();
        auto pData = pool.Get_Data_By_Handle(hComponent);
        pData->hObject = hObject;

        Initialize_Component_Data(TProxy::ComponentType, hComponent);

        return hComponent;
    }

    template <typename TProxy>
    void Remove_Component_Inner(CComponent_Pool<TProxy>& pool, COMPONENT_HANDLE hComponent)
    {
        pool.Deallocate(hComponent);
    }

    template <typename TProxy>
    void Set_Enable_Inner(CComponent_Pool<TProxy>& pool, COMPONENT_HANDLE hComponent, _bool bEnable)
    {
        pool.Get_Data_By_Handle(hComponent)->bEnable = bEnable;
    }

private :
    HRESULT Initialize_From_Spec_Collider(COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* spec);
    HRESULT Initialize_From_Spec_Rigidbody(COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* spec);
    HRESULT Initialize_From_Spec_SpringJoint(COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* spec);

    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec_Collider(COMPONENT_HANDLE hComponent);
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec_Rigidbody(COMPONENT_HANDLE hComponent);
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec_SpringJoint(COMPONENT_HANDLE hComponent);

    HRESULT Initialize_Component_Data(COMPONENT_TYPE eComType, COMPONENT_HANDLE h);

public :
    _bool       Detect_Raycast(RAY& tRay, RAYCAST_HITS& outHits);

private:
    CComponent_Pool<CCollider>          m_ColliderPool;
    CComponent_Pool<CRigidbody>         m_RigidbodyPool;
    CComponent_Pool<CSpringJoint>       m_SpringJointPool;

    std::vector<COLLIDER_PROXY_DATA>    m_ActivatedColliders;   /* Transient */
    std::unordered_set<PAIR_KEY, PAIR_KEY_HASHER> m_CurPair;    /* 충돌 이벤트 */
    std::unordered_set<PAIR_KEY, PAIR_KEY_HASHER> m_prevPair;

    CTransform_Processor*                       m_pTransformProcessor{};
    std::unique_ptr<CCollision_Detector>        m_upCollision_Detector{};
    std::unique_ptr<CCollider_Proxy_Builder>    m_upCollider_Builder{};
    std::unique_ptr<CRigidbody_Builder>         m_upRigidbody_Builder{};
    std::unique_ptr<CSolver>                    m_upSolver{};
    std::unique_ptr<CDebug_Renderer>            m_upDebugRenderer{};

    const _float    m_fGravity = -9.81f;
    const _float3   m_vGravity = { 0.f, m_fGravity, 0.f };

private :
    void    Process_SpringJoints(_float fDT);

private :
    void    Accumulate_Forces();
    void    Integrate_Forces(_float fDT);
    void    Apply_Damping(_float fDT);
    void    Integrate_Velocities(_float fDT);
    void    Process_Collision(vector<CONTACT_DESC>& outContacts);
    void    Reset_Kinematic_Velocities();

    void    Invoke_CollisionEvent();
    void    Apply_RotationLock(RIGIDBODY_DATA& data);
    void    Apply_PositionLock(RIGIDBODY_DATA& data);

public :
    static std::unique_ptr<CPhysics_Processor> Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
};


NS_END
