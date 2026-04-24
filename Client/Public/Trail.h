#pragma once
#include "Client_Define.h"
#include "Line.h"

#include <deque>
#include <vector>
#include <memory>

NS_BEGIN(Client)

class CTrail final
{
    typedef struct tagTrailPoint
    {
        _float3 vPosition{};
        _float3 vRight{};
        _float  fWidth = 0.2f;
        _float4 vColor = { 1.f, 1.f, 1.f, 1.f };
        _float  fLifeTime = 0.f;
    } TRAIL_POINT;

public:
    CTrail();
    ~CTrail();

public:
    void Initialize(const _float3& vStartPoint, const _float3& vRight);
    void Update(_float fDT);

    void Set_StartPoint(const _float3& vStartPoint, const _float3& vRight, WIDTH_TYPE eWidth = WIDTH_TYPE::NORMAL);
    void Set_Color(const _float4& vColor);

private:
    void Expire_Points();
    void Build_RenderPoints();
    void Push_Point(const _float3& vPosition, const _float3& vRight, _float fWidth);
    _bool Can_Spawn_Point(const _float3& vPosition) const;

private:
    std::deque<TRAIL_POINT>         m_deqTrailPoints;
    std::vector<LINE_POINT>         m_vecRenderPoints;
    std::unique_ptr<CLine>          m_pLine;

    _float3                         m_vStartPoint{};
    _float3                         m_vDefaultRight{ 1.f, 0.f, 0.f };
    _float4                         m_vColor{ 0.5f, 0.5f, 0.5f, 0.7f };

    _int                            m_iNumPoints = 30;
    _float                          m_fPointLifeTime = 0.7f;
    _float                          m_fMinSpawnDist = 0.1f;

    _float                          m_fStratOffsetY = 0.5f;

    _float                          m_fWidthTypes[To<_uint>(WIDTH_TYPE::END)] = {};

public:
    static std::unique_ptr<CTrail> Create();
};

NS_END
