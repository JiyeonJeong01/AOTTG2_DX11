#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

namespace Math
{
    // Load/Store
    inline _vector Load(const _float2& v) { return XMLoadFloat2(&v); }
    inline _vector Load(const _float3& v) { return XMLoadFloat3(&v); }
    inline _vector Load(const _float4& v) { return XMLoadFloat4(&v); }
    inline _matrix Load(const _float4x4& m) { return XMLoadFloat4x4(&m); }

    inline void Store(_float2& out, _vector v) { XMStoreFloat2(&out, v); }
    inline void Store(_float3& out, _vector v) { XMStoreFloat3(&out, v); }
    inline void Store(_float4& out, _vector v) { XMStoreFloat4(&out, v); }
    inline void Store(_float4x4& out, _matrix m) { XMStoreFloat4x4(&out, m); }

    // Identity
    inline _matrix IdentityM()
    {
        return XMMatrixIdentity();
    }

    inline _float4x4 Identity()
    {
        _float4x4 m;
        XMStoreFloat4x4(&m, XMMatrixIdentity());
        return m;
    }

    inline void Set_IdentityM(_float4x4& out)
    {
        XMStoreFloat4x4(&out, XMMatrixIdentity());
    }

    // 연산용 Matrix Builders
    inline _matrix TranslationM(float x, float y, float z)
    {
        return XMMatrixTranslation(x, y, z);
    }

    inline _matrix TranslationM(const _float3& t)
    {
        return XMMatrixTranslation(t.x, t.y, t.z);
    }

    inline _matrix ScaleM(float x, float y, float z)
    {
        return XMMatrixScaling(x, y, z);
    }

    inline _matrix ScaleM(const _float3& s)
    {
        return XMMatrixScaling(s.x, s.y, s.z);
    }

    // 라디안 기준 회전
    inline _matrix RotationXM(float fRadX) { return XMMatrixRotationX(fRadX); }
    inline _matrix RotationYM(float fRadY) { return XMMatrixRotationY(fRadY); }
    inline _matrix RotationZM(float fRadZ) { return XMMatrixRotationZ(fRadZ); }

    inline _matrix RotationRollPitchYawM(float pitch, float yaw, float roll)
    {
        return XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
    }

    inline _matrix RotationRollPitchYawM(const _float3& r)
    {
        return XMMatrixRotationRollPitchYaw(r.x, r.y, r.z);
    }

    // 저장용 Matrix Builders
    inline _float4x4 Translation(float x, float y, float z)
    {
        _float4x4 m;
        XMStoreFloat4x4(&m, XMMatrixTranslation(x, y, z));
        return m;
    }

    inline _float4x4 Translation(const _float3& t)
    {
        _float4x4 m;
        XMStoreFloat4x4(&m, XMMatrixTranslation(t.x, t.y, t.z));
        return m;
    }

    inline _float4x4 Scale(float x, float y, float z)
    {
        _float4x4 m;
        XMStoreFloat4x4(&m, XMMatrixScaling(x, y, z));
        return m;
    }

    inline _float4x4 Scale(const _float3& s)
    {
        _float4x4 m;
        XMStoreFloat4x4(&m, XMMatrixScaling(s.x, s.y, s.z));
        return m;
    }

    inline _float4x4 RotationX(float rad)
    {
        _float4x4 m;
        XMStoreFloat4x4(&m, XMMatrixRotationX(rad));
        return m;
    }

    inline _float4x4 RotationY(float rad)
    {
        _float4x4 m;
        XMStoreFloat4x4(&m, XMMatrixRotationY(rad));
        return m;
    }

    inline _float4x4 RotationZ(float rad)
    {
        _float4x4 m;
        XMStoreFloat4x4(&m, XMMatrixRotationZ(rad));
        return m;
    }

    inline _float4x4 RotationRollPitchYaw(float pitch, float yaw, float roll)
    {
        _float4x4 m;
        XMStoreFloat4x4(&m, XMMatrixRotationRollPitchYaw(pitch, yaw, roll));
        return m;
    }

    inline _float4x4 RotationRollPitchYaw(const _float3& r)
    {
        _float4x4 m;
        XMStoreFloat4x4(&m, XMMatrixRotationRollPitchYaw(r.x, r.y, r.z));
        return m;
    }

    // SRT
    inline _matrix SRTM(const _float3& s, const _float3& r, const _float3& t)
    {
        const _matrix S = ScaleM(s);
        const _matrix R = RotationRollPitchYawM(r);
        const _matrix T = TranslationM(t);
        return S * R * T;
    }

    inline _float4x4 SRT(const _float3& s, const _float3& r, const _float3& t)
    {
        _float4x4 out;
        XMStoreFloat4x4(&out, SRTM(s, r, t));
        return out;
    }

    inline void Set_SRT(_float4x4& out, const _float3& s, const _float3& r, const _float3& t)
    {
        XMStoreFloat4x4(&out, SRTM(s, r, t));
    }

    // Transpose (상수버퍼 업로드 규칙에 맞춰 사용)
    inline _matrix TransposeM(_matrix m)
    {
        return XMMatrixTranspose(m);
    }

    inline _float4x4 Transpose(const _float4x4& m)
    {
        _float4x4 out;
        XMStoreFloat4x4(&out, XMMatrixTranspose(XMLoadFloat4x4(&m)));
        return out;
    }

    inline void Set_Transpose(_float4x4& out, const _float4x4& m)
    {
        XMStoreFloat4x4(&out, XMMatrixTranspose(XMLoadFloat4x4(&m)));
    }

    // Safe index helper for enum class -> size_t
    template<typename TEnum>
    constexpr size_t ToIndex(TEnum e)
    {
        return static_cast<size_t>(e);
    }

    inline _matrix RotationQuaternionM(const _vector& q)
    {
        return XMMatrixRotationQuaternion(q);
    }

    inline _matrix RotationQuaternionM(const _float4& q)
    {
        return XMMatrixRotationQuaternion(
            XMVectorSet(q.x, q.y, q.z, q.w));
    }

    inline _float3 TransformNormal(const _float3& v, _fmatrix m)
    {
        const XMVECTOR vv = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v));
        const XMVECTOR r = XMVector3TransformNormal(vv, m);

        _float3 out{};
        XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&out), r);
        return out;
    }

    inline _float3 Normalize(const _float3& v)
    {
        using namespace DirectX;
        XMVECTOR vv = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v));
        vv = XMVector3Normalize(vv);

        _float3 out{};
        XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&out), vv);
        return out;
    }

    inline _float3 TransformCoord(const _float3& p, _matrix m)
    {
        using namespace DirectX;
        XMVECTOR vp = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&p));
        XMVECTOR r = XMVector3TransformCoord(vp, m);

        _float3 out{};
        XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&out), r);
        return out;
    }

    inline _matrix Matrix_Inverse(_matrix m)
    {
        using namespace DirectX;
        return XMMatrixInverse(nullptr, m);
    }

    inline _matrix Matrix_LookAtLH(const _float3& vEye, const _float3& vAt, const _float3& vUp)
    {
        using namespace DirectX;

        const XMVECTOR eye = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&vEye));
        const XMVECTOR at = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&vAt));
        const XMVECTOR up = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&vUp));
        return XMMatrixLookAtLH(eye, at, up);
    }

    inline _matrix Matrix_PerspectiveFovLH(_float fFovyRad, _float fAspect, _float fNear, _float fFar)
    {
        return XMMatrixPerspectiveFovLH(fFovyRad, fAspect, fNear, fFar);
    }

    // Vector
    inline _vector Zero_Vec() { return XMVectorZero(); }
    inline _vector Right_Vec() { return XMVectorSet(1.f, 0.f, 0.f, 0.f); }
    inline _vector Up_Vec() { return XMVectorSet(0.f, 1.f, 0.f, 0.f); }
    inline _vector Look_Vec() { return XMVectorSet(0.f, 0.f, 1.f, 0.f); }

    inline _float3 QuaternionToEulerRad(const _vector& q)
    {
        _float3 euler{};

        // Pitch (X)
        const float sinp = 2.f * (XMVectorGetW(q) * XMVectorGetX(q)
            - XMVectorGetZ(q) * XMVectorGetY(q));
        if (fabsf(sinp) >= 1.f)
            euler.x = copysignf(XM_PIDIV2, sinp);
        else
            euler.x = asinf(sinp);

        // Yaw (Y)
        const float siny = 2.f * (XMVectorGetW(q) * XMVectorGetY(q)
            + XMVectorGetX(q) * XMVectorGetZ(q));
        const float cosy = 1.f - 2.f * (XMVectorGetX(q) * XMVectorGetX(q)
            + XMVectorGetY(q) * XMVectorGetY(q));
        euler.y = atan2f(siny, cosy);

        // Roll (Z)
        const float sinr = 2.f * (XMVectorGetW(q) * XMVectorGetZ(q)
            + XMVectorGetX(q) * XMVectorGetY(q));
        const float cosr = 1.f - 2.f * (XMVectorGetY(q) * XMVectorGetY(q)
            + XMVectorGetZ(q) * XMVectorGetZ(q));
        euler.z = atan2f(sinr, cosr);

        return euler; // radian
    }

    // Quaternion (_vector) -> Euler (degree)
    inline _float3 QuaternionToEulerDeg(const _vector& q)
    {
        const _float3 rad = QuaternionToEulerRad(q);
        return {
            XMConvertToDegrees(rad.x),
            XMConvertToDegrees(rad.y),
            XMConvertToDegrees(rad.z)
        };
    }

    // 저장용 overload
    inline _float3 QuaternionToEulerDeg(const _float4& q)
    {
        return QuaternionToEulerDeg(XMVectorSet(q.x, q.y, q.z, q.w));
    }

    inline _float4 EulerRadToQuaternion(const _float3& eulerRad)
    {
        const _vector q = XMQuaternionRotationRollPitchYaw(
            eulerRad.x, eulerRad.y, eulerRad.z);

        _float4 out{};
        XMStoreFloat4(&out, XMQuaternionNormalize(q));
        return out;
    }

    inline _float4 EulerDegToQuaternion(const _float3& eulerDeg)
    {
        _float3 rad{
            XMConvertToRadians(eulerDeg.x),
            XMConvertToRadians(eulerDeg.y),
            XMConvertToRadians(eulerDeg.z)
        };

        return EulerRadToQuaternion(rad);
    }


    inline _float3 Store(const _vector& v)
    {
        _float3 out{};
        XMStoreFloat3(&out, v);
        return out;
    }

    inline _float3 Zero3()
    {
        return { 0.f, 0.f, 0.f };
    }
}

NS_END
