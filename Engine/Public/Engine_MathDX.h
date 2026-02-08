#pragma once
#include "Engine_Define.h"

NS_BEGIN(MathDX)

inline _vector  Load(const _float2& v) { return XMLoadFloat2(&v); }
inline void     Store(_float2& out, _fvector v) { XMStoreFloat2(&out, v); }

inline _vector  Load(const _float3& v) { return XMLoadFloat3(&v); }
inline void     Store(_float3& out, _fvector v) { XMStoreFloat3(&out, v); }

inline _vector  Load(const _float4& v) { return XMLoadFloat4(&v); }
inline void     Store(_float4& out, _fvector v) { XMStoreFloat4(&out, v); }

inline _matrix  Load(const _float4x4& m) { return XMLoadFloat4x4(&m); }
inline void     Store(_float4x4& out, _fmatrix m) { XMStoreFloat4x4(&out, m); }

inline void     Store(_float2* out, _fvector v) { XMStoreFloat2(out, v); }
inline _vector  Load(const _float2* v) { return XMLoadFloat2(v); }

inline void     Store(_float3* out, _fvector v) { XMStoreFloat3(out, v); }
inline _vector  Load(const _float3* v) { return XMLoadFloat3(v); }

inline void     Store(_float4* out, _fvector v) { XMStoreFloat4(out, v); }
inline _vector  Load(const _float4* v) { return XMLoadFloat4(v); }

inline void     Store(_float4x4* out, _fmatrix m) { XMStoreFloat4x4(out, m); }
inline _matrix  Load(const _float4x4* m) { return XMLoadFloat4x4(m); }

inline _matrix  Identity() { return XMMatrixIdentity(); }
inline static const _float4x4 s_dummy_float4x4{ };

inline _vector  World_Right() { return { 1.f, 0.f, 0.f, 0.f }; }
inline _vector  World_Up() { return { 0.f, 1.f, 0.f, 0.f }; }
inline _vector  World_Look() { return { 0.f, 0.f, 1.f, 0.f }; }
inline _vector  World_Origin() { return { 0.f, 0.f, 0.f, 1.f }; }

NS_END
