#include "Trail.h"

#include "GameInstance.h"
#include "Engine_Math.h"

using namespace Client;

CTrail::CTrail()
{
}

CTrail::~CTrail()
{
}

void CTrail::Initialize(const _float3& vStartPoint, const _float3& vRight)
{
    m_vStartPoint = vStartPoint;

    m_deqTrailPoints.clear();

    for (_int i = 0; i < m_iNumPoints; ++i)
    {
        TRAIL_POINT tPoint{};
        tPoint.vPosition = vStartPoint;
        tPoint.vRight = vRight;
        tPoint.vColor = m_vColor;
        tPoint.fLifeTime = 0.f;

        m_deqTrailPoints.push_back(tPoint);
    }

    m_fWidthTypes[To<_uint>(WIDTH_TYPE::NONE)] = 0.f;
    m_fWidthTypes[To<_uint>(WIDTH_TYPE::THIN)] = 0.08f;
    m_fWidthTypes[To<_uint>(WIDTH_TYPE::NORMAL)] = 0.2f;
    m_fWidthTypes[To<_uint>(WIDTH_TYPE::BOLD)] = 0.5f;
}

void CTrail::Update(_float fDT)
{
    if (m_deqTrailPoints.empty())
        return;

    for (auto& tPoint : m_deqTrailPoints)
        tPoint.fLifeTime += fDT;

    Expire_Points();

    if (m_deqTrailPoints.size() < 2)
        return;

    Build_RenderPoints();

    if (m_vecRenderPoints.size() < 2)
        return;

    if (m_pLine == nullptr)
        return;

    m_pLine->Update_Trail(m_vecRenderPoints.data(), static_cast<_uint>(m_vecRenderPoints.size()));
    m_pLine->Submit_Trail();
}

void CTrail::Set_StartPoint(const _float3& vStartPoint, const _float3& vRight, WIDTH_TYPE eWidth)
{
    if (false == Can_Spawn_Point(vStartPoint))
        return;

    _float fWidth = m_fWidthTypes[To<_uint>(eWidth)];

    Push_Point(vStartPoint, vRight, fWidth);
}

void CTrail::Set_Color(const _float4& vColor)
{
    m_vColor = vColor;
}

void CTrail::Expire_Points()
{
    /* 수명이 지난 점들은 없애기 */
    while (false == m_deqTrailPoints.empty())
    {
        if (m_deqTrailPoints.front().fLifeTime < m_fPointLifeTime)
            break;

        m_deqTrailPoints.pop_front();
    }
}

void CTrail::Build_RenderPoints()
{
    /* m_deqTrailPoints : 실제로 저장된 원본 경로 */
    /* m_vecRenderPoints : 렌더링용 고정 개수 */
    /* 원본 데이터를 렌더용 데이터로 변환한다. */

    m_vecRenderPoints.clear();

    const size_t iSrcCount = m_deqTrailPoints.size();
    if (iSrcCount < 2)
        return;

    const _int iDstCount = m_iNumPoints;
    if (iDstCount < 2)
        return;

    /* 거리 누적 계산 : 일정 간격으로 트레일을 표현하기 위하여 거리 기반으로 계산한다. */
    std::vector<_float> vecAccumDist(iSrcCount, 0.f);
    _float fTotalLength = 0.f;

    for (size_t i = 1; i < iSrcCount; ++i)
    {
        const _float3& vPrev = m_deqTrailPoints[i - 1].vPosition;
        const _float3& vCur = m_deqTrailPoints[i].vPosition;

        const _float fDx = vCur.x - vPrev.x;
        const _float fDy = vCur.y - vPrev.y;
        const _float fDz = vCur.z - vPrev.z;

        const _float fSegLen = sqrtf(fDx * fDx + fDy * fDy + fDz * fDz);
        fTotalLength += fSegLen;
        vecAccumDist[i] = fTotalLength;
    }

    /* 거의 움직이지 않은 경우 보간 불가능 */
    if (fTotalLength <= 1e-6f)
    {
        m_vecRenderPoints.resize(iDstCount);

        const TRAIL_POINT& tLast = m_deqTrailPoints.back();
        for (_int i = 0; i < iDstCount; ++i)
        {
            m_vecRenderPoints[i].vPosition = tLast.vPosition;
            m_vecRenderPoints[i].vRight = tLast.vRight;
            m_vecRenderPoints[i].fWidth = tLast.fWidth;
            m_vecRenderPoints[i].vColor = tLast.vColor;
        }

        return;
    }

    /* 목표 거리 계산 */
    m_vecRenderPoints.reserve(iDstCount);
    
    for (_int i = 0; i < iDstCount; ++i)
    {
        const _float fRatio = (_float)i / (_float)(iDstCount - 1);
        const _float fTargetDist = fTotalLength * fRatio;

        size_t iSeg = 1;
        for (; iSeg < iSrcCount; ++iSeg)
        {
            if (vecAccumDist[iSeg] >= fTargetDist)
                break;
        }

        if (iSeg >= iSrcCount)
            iSeg = iSrcCount - 1;

        const size_t iPrev = iSeg - 1;

        const _float fSegStart = vecAccumDist[iPrev];
        const _float fSegEnd = vecAccumDist[iSeg];
        const _float fSegLen = fSegEnd - fSegStart;

        _float fLerp = 0.f;
        if (fSegLen > 1e-6f)
            fLerp = (fTargetDist - fSegStart) / fSegLen;

        const TRAIL_POINT& tA = m_deqTrailPoints[iPrev];
        const TRAIL_POINT& tB = m_deqTrailPoints[iSeg];

        LINE_POINT tRenderPoint{};

        tRenderPoint.vPosition.x = tA.vPosition.x + (tB.vPosition.x - tA.vPosition.x) * fLerp;
        tRenderPoint.vPosition.y = tA.vPosition.y + (tB.vPosition.y - tA.vPosition.y) * fLerp;
        tRenderPoint.vPosition.z = tA.vPosition.z + (tB.vPosition.z - tA.vPosition.z) * fLerp;

        _float3 vRight{};
        vRight.x = tA.vRight.x + (tB.vRight.x - tA.vRight.x) * fLerp;
        vRight.y = tA.vRight.y + (tB.vRight.y - tA.vRight.y) * fLerp;
        vRight.z = tA.vRight.z + (tB.vRight.z - tA.vRight.z) * fLerp;

        const _vector vRightXM = Math::Load(vRight);
        if (Math::Get_X(XMVector3LengthSq(vRightXM)) > 1e-6f)
        {
            _float3 vNormRight{};
            Math::Store(vNormRight, Math::Normalize(vRightXM));
            tRenderPoint.vRight = vNormRight;
        }
        else
        {
            tRenderPoint.vRight = m_vDefaultRight;
        }

        tRenderPoint.fWidth = tA.fWidth + (tB.fWidth - tA.fWidth) * fLerp;

        tRenderPoint.vColor.x = tA.vColor.x + (tB.vColor.x - tA.vColor.x) * fLerp;
        tRenderPoint.vColor.y = tA.vColor.y + (tB.vColor.y - tA.vColor.y) * fLerp;
        tRenderPoint.vColor.z = tA.vColor.z + (tB.vColor.z - tA.vColor.z) * fLerp;
        tRenderPoint.vColor.w = tA.vColor.w + (tB.vColor.w - tA.vColor.w) * fLerp;

        m_vecRenderPoints.push_back(tRenderPoint);
    }

    if (false == m_vecRenderPoints.empty())
    {
        const size_t iLast = m_vecRenderPoints.size() - 1;

        const _float fInvLife = (m_fPointLifeTime > 1e-6f) ? (1.f / m_fPointLifeTime) : 0.f;

        for (size_t i = 0; i < m_vecRenderPoints.size(); ++i)
        {
            const _float fRatio = (_float)i / (_float)iLast;

            const _float fAlphaFade = fRatio;
            m_vecRenderPoints[i].vColor.w *= fAlphaFade;

            const _float fWidthFade = 0.35f + 0.65f * fRatio;
            m_vecRenderPoints[i].fWidth *= fWidthFade;
        }

        (void)fInvLife;
    }
}

void CTrail::Push_Point(const _float3& vPosition, const _float3& vRight, _float fWidth)
{
    TRAIL_POINT tPoint{};
    tPoint.vPosition = vPosition;
    tPoint.vRight = vRight;
    tPoint.fWidth = fWidth;
    tPoint.vColor = m_vColor;
    tPoint.fLifeTime = 0.f;

    m_deqTrailPoints.push_back(tPoint);
}

_bool CTrail::Can_Spawn_Point(const _float3& vPosition) const
{
    if (m_deqTrailPoints.empty())
        return true;

    const _float3& vLast = m_deqTrailPoints.back().vPosition;

    const _float fDx = vPosition.x - vLast.x;
    const _float fDy = vPosition.y - vLast.y;
    const _float fDz = vPosition.z - vLast.z;

    const _float fDistSq = fDx * fDx + fDy * fDy + fDz * fDz;
    return fDistSq >= (m_fMinSpawnDist * m_fMinSpawnDist);
}

std::unique_ptr<CTrail> CTrail::Create()
{
    auto pInstance = std::make_unique<CTrail>();

    pInstance->m_pLine = GAME_INSTANCE.Load_LineMesh(pInstance->m_iNumPoints, 0.2f, LINE_TYPE::TRAIL);
    IF_NULL_RETURN_MSG_BREAK(pInstance->m_pLine, nullptr, "Trail line create failed.");
    pInstance->Initialize(pInstance->m_vStartPoint, pInstance->m_vDefaultRight);

    return pInstance;
}
