float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;


vector g_BaseColor = {1.f, 1.f, 1.f, 0.7f};

SamplerState DefaultSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
};

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float4 p = float4(In.vPosition, 1);
    p = mul(p, g_WorldMatrix);
    p = mul(p, g_ViewMatrix);
    p = mul(p, g_ProjMatrix);
    Out.vPosition = p;
    Out.vTexcoord = In.vTexcoord;
    return Out;
}

float2 g_vVelocityDir = float2(1.f, 0.f);
float g_fLineThickness = 0.01f;
float g_fLineCount   = 20.f;
float g_fIntensity = 1.f;
float g_fTime = 0.f;

// float4 PS_MAIN(VS_OUT In) : SV_TARGET
// {
//     float2 vUV = In.vTexcoord;
//     float2 vCentered = vUV - float2(0.5f, 0.5f);

//     float fRadius = length(vCentered);
//     if (fRadius <= 0.001f)
//         discard;

//     float2 vRadialDir = normalize(vCentered);
//     float2 vMoveDir = normalize(g_vVelocityDir);

//     /* 반대방향일수록 1, 정방향일수록 0 */
//     float fSide = saturate(dot(vRadialDir, -vMoveDir) * 0.5f + 0.5f);

//     /* 반대방향일수록 더 진하게 */
//     float fDirWeight = lerp(0.30f, 0.85f, fSide);

//     /* 반대방향일수록 라인이 더 많아 보이게 */
//     float fLocalLineCount = lerp(g_fLineCount * 0.85f, g_fLineCount * 1.35f, fSide);

//     float fAngle = atan2(vRadialDir.y, vRadialDir.x);

//     /* 시간에 따라 라인 패턴이 회전하듯 조금씩 이동 */
//     float fAngleAnim = fAngle + g_fTime * 0.35f;

//     float fRepeat = frac((fAngleAnim / 6.28318f) * fLocalLineCount);
//     float fLine = abs(fRepeat - 0.5f);

//     /* 반대방향일수록 라인도 조금 더 굵고 존재감 있게 */
//     float fLocalThickness = lerp(g_fLineThickness * 0.85f, g_fLineThickness * 1.25f, fSide);
//     if (fLine >= fLocalThickness)
//         discard;

//     float fLineIndex = floor((fAngleAnim / 6.28318f) * fLocalLineCount);

//     /* 시간에 따라 라인별 랜덤값이 3프레임처럼 조금씩 바뀌게 */
//     float fFrame = floor(g_fTime * 12.f) % 3.f;
//     float fRandom = frac(sin((fLineIndex + fFrame * 17.f) * 12.9898f) * 43758.5453f);

//     /* 반대방향일수록 중심 더 가까이서 시작 = 더 길어 보임 */
//     float fStartRadiusBase = lerp(0.45f, 0.01f, fSide);
//     float fStartRadiusJitter = lerp(0.04f, 0.12f, fRandom);
//     float fStartRadius = fStartRadiusBase + fStartRadiusJitter;

//     if (fRadius < fStartRadius)
//         discard;

//     /* 반대방향일수록 더 멀리까지 살아남음. 더 길어 보임 */
//     float fEndRadius = lerp(0.68f, 0.98f, fSide);
//     if (fRadius > fEndRadius)
//         discard;

//     /* 중심은 약하고 바깥으로 갈수록 강하게 */
//     float fRadiusWeight = saturate((fRadius - fStartRadius) / max(0.001f, (fEndRadius - fStartRadius)));

//     /* 시간에 따른 미세한 깜빡임/흐름 */
//     float fPulse = lerp(0.85f, 1.10f, 0.5f + 0.5f * sin(g_fTime * 18.f + fLineIndex * 1.7f));

//     float fAlpha = fDirWeight * fRadiusWeight * fPulse * g_fIntensity;

//     return float4(g_BaseColor.rgb, g_BaseColor.a * fAlpha);
// }

float4 PS_MAIN(VS_OUT In) : SV_TARGET
{
    float2 vUV = In.vTexcoord;
    float2 vCentered = vUV - float2(0.5f, 0.5f);

    float2 vMoveDir = normalize(g_vVelocityDir);
    float2 vPerp = float2(-vMoveDir.y, vMoveDir.x);

    float fAlong = dot(vCentered, -vMoveDir);
    float fSide = dot(vCentered, vPerp);

    if (fAlong < 0.05f)
        discard;

    float fRadiusSq = dot(vCentered, vCentered);
    if (fRadiusSq > 0.95f * 0.95f)
        discard;

    float fRepeat = frac((fSide + g_fTime * 0.4f) * g_fLineCount);
    float fLine = abs(fRepeat - 0.5f);

    if (fLine > g_fLineThickness)
        discard;

    float fAlpha = saturate(fAlong * 2.0f) * g_fIntensity;

    return float4(g_BaseColor.rgb, g_BaseColor.a * fAlpha);
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetVertexShader( CompileShader(vs_5_0, VS_MAIN()) );
        SetPixelShader(  CompileShader(ps_5_0, PS_MAIN()) );
    }
}
