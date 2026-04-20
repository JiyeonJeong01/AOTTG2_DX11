#pragma once
#include "Client_Define.h"

NS_BEGIN(Client)

class CEasingFunction final
{
public:
    static _float Clamp01(_float t)
    {
        if (t < 0.f) return 0.f;
        if (t > 1.f) return 1.f;
        return t;
    }

    static _float Lerp(_float a, _float b, _float t)
    {
        return a + (b - a) * t;
    }

    static _float2 Lerp(const _float2& a, const _float2& b, _float t)
    {
        return _float2(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t
        );
    }

    static _float3 Lerp(const _float3& a, const _float3& b, _float t)
    {
        return _float3(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t
        );
    }

    static _float EaseOutQuad(_float t)
    {
        t = Clamp01(t);
        return 1.f - (1.f - t) * (1.f - t);
    }

    static _float EaseOutCubic(_float t)
    {
        t = Clamp01(t);
        const _float inv = 1.f - t;
        return 1.f - inv * inv * inv;
    }

    static _float EaseOutBack(_float t)
    {
        t = Clamp01(t);
        const _float c1 = 1.70158f;
        const _float c3 = c1 + 1.f;
        const _float p = t - 1.f;
        return 1.f + c3 * p * p * p + c1 * p * p;
    }

    static _float SmoothDampAlpha(_float sharpness, _float fDT)
    {
        if (sharpness <= 0.f)
            return 1.f;

        const _float t = 1.f - expf(-sharpness * fDT);
        return Clamp01(t);
    }

    static _float DampedLerp(_float current, _float target, _float sharpness, _float fDT)
    {
        return Lerp(current, target, SmoothDampAlpha(sharpness, fDT));
    }

    static _float2 DampedLerp(const _float2& current, const _float2& target, _float sharpness, _float fDT)
    {
        return Lerp(current, target, SmoothDampAlpha(sharpness, fDT));
    }
};

NS_END
