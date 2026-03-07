#pragma once
#include "Collider.h"

NS_BEGIN(Engine)

class CPhysics_Processor;

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
    _bool	Detect_SphereCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB);
    _bool	Detect_SpherePlaneCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB);
    _bool	Detect_BoxPlaneCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB);
    _bool	Detect_BoxSphereCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB);
    _bool	Detect_BoxCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB);

    class CPhysics_Processor* m_pPhysics_Processor{};
    class CTransform_Processor* m_pTransform_Processor{};

private :
    void Register_DetectTable();
    void Fill_ContactInfo(CONTACT_DESC& outContact);

private :
    std::array<std::array<DetectFunc, To<size_t>(SHAPE::END)>, To<size_t>(SHAPE::END)> m_DetectTable{};

public :
    static std::unique_ptr<CCollision_Detector> Create(CPhysics_Processor* pPhysics);

};

NS_END
