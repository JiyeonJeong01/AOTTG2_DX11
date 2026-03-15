#pragma once
#include "Collider.h"

#pragma region FORWARD DECLARATION
NS_BEGIN(Engine)
class CPhysics_Processor;
typedef struct tagRay RAY;
typedef struct tagRaycastHit RAYCAST_HIT;
typedef struct tagRaycastHits RAYCAST_HITS;
NS_END
#pragma endregion

NS_BEGIN(Engine)
class CCollision_Detector final
{
    using DetectFunc = std::function<_bool(CONTACT_DESC*, COLLIDER_PROXY_DATA*, COLLIDER_PROXY_DATA*)>;
public :
    CCollision_Detector(CPhysics_Processor* pPhysics);
    ~CCollision_Detector();

public :
    HRESULT Initialize();

    /* Broad Phase */
    void    Generate_BroadPhase_Pairs(const vector<COLLIDER_PROXY_DATA>& allColliders, _Out_ vector< COLLIDER_PAIR>& outPair);

    /* Narrow Phase */
    void	Process_NarrowPhase(const vector<COLLIDER_PAIR>& pairs, vector<CONTACT_DESC>& outContacts);

    /* Raycast */
    _bool   Detect_Raycast(RAY& tRay,
                            const std::vector<COLLIDER_PROXY_DATA>& AllColliders,
                            RAYCAST_HITS& outHits);

    class CPhysics_Processor* m_pPhysics_Processor{};
    class CTransform_Processor* m_pTransform_Processor{};

private :
    void    Register_DetectTable();
    void    Fill_ContactInfo(CONTACT_DESC& outContact);

    /* Narrow phase */
    _bool	Detect_SphereCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB);
    _bool	Detect_SpherePlaneCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB);
    _bool	Detect_BoxPlaneCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB);
    _bool	Detect_BoxSphereCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB);
    _bool	Detect_BoxCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB);

    /* Raycast */
    _bool   Detect_RayBox(RAYCAST_HIT* pOutHit, RAY& tRay, const COLLIDER_PROXY_DATA& tData);
    _bool   Detect_RaySphere(RAYCAST_HIT* pOutHit, RAY& tRay, const COLLIDER_PROXY_DATA& tData);
    _bool   Detect_RayPlane(RAYCAST_HIT* pOutHit, RAY& tRay, const COLLIDER_PROXY_DATA& tData);

private :
    std::array<std::array<DetectFunc, To<size_t>(SHAPE::END)>, To<size_t>(SHAPE::END)> m_DetectTable{};

public :
    static std::unique_ptr<CCollision_Detector> Create(CPhysics_Processor* pPhysics);

};

NS_END
