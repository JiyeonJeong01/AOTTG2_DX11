#include "Engine_Define.h"

NS_BEGIN(Engine)

class CPhysics_Processor;

typedef struct tagRay
{
    _float3     vOrigin{};
    _float3     vDir{};   /* normalize */
    _float      fMaxDist = 9999.f;
    _float      fMinDist = 0;

} RAY;

typedef struct tagRaycastHit
{
    OBJECT_HANDLE   hObject{};

    _float3         vHitPos{};
    _float          fDist{};
} RAYCAST_HIT;

typedef struct tagRaycastHits
{
    RAYCAST_HIT         primaryHit{};
    vector<RAYCAST_HIT> allHits;
    _uint               iNumHits{};
} RAYCAST_HITS;

class ENGINE_DLL CRaycast final
{
public:
    static _bool    Intersect_Ray(RAYCAST_HITS& outHits, RAY& tRAY, const RAY& ray);

    /**
     * \param ptGame 반드시 SYS_INPUT->Get_GameMousePos()를 사용하여 전달할 것.
     */
    static _bool    Intersect_Ray(RAYCAST_HITS& outHits, RAY& tRAY, const POINT& ptGame, CPhysics_Processor* pPhysics);
    static RAY      Build_Ray_From_Screen(float mouseX, float mouseY, float vpW, float vpH,
                                        const _float4x4& matView, const _float4x4& matProj);
};

NS_END
