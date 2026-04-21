float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4 g_ViewMatrixInverse, g_ProjMatrixInverse;

Texture2D g_SceneTexture;
Texture2D g_DepthTexture;

vector g_vCamPosition;

float4 g_vFogColor;
float4 g_vFogParams; // x=start, y=end, z=density, w=enable

float fFar = 1000.f;

sampler DefaultSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = clamp;
    AddressV = clamp;
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

    float4x4 matWV = mul(g_WorldMatrix, g_ViewMatrix);
    float4x4 matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;

    return Out;
}

float4 Reconstruct_WorldPos(float2 vTexcoord)
{
    float4 vDepthDesc = g_DepthTexture.Sample(DefaultSampler, vTexcoord);
    float fViewZ = vDepthDesc.y * fFar;

    float4 vPos;
    vPos.x = vTexcoord.x * 2.f - 1.f;
    vPos.y = vTexcoord.y * -2.f + 1.f;
    vPos.z = vDepthDesc.x;
    vPos.w = 1.f;

    vPos *= fViewZ;
    vPos = mul(vPos, g_ProjMatrixInverse);
    vPos = mul(vPos, g_ViewMatrixInverse);

    return vPos;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

PS_OUT PS_MAIN_FOG(PS_IN In)
{
    PS_OUT Out;

    float4 vScene = g_SceneTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vColor = vScene;

    /* x : start, y : end, z : density, w : enable */
    if (g_vFogParams.w > 0.5f)
    {
        float4 vWorldPos = Reconstruct_WorldPos(In.vTexcoord);

        float3 vFogVec = vWorldPos.xyz - g_vCamPosition.xyz;
        float fFogDist = length(vFogVec);

        float fDistMask = smoothstep(g_vFogParams.x, g_vFogParams.y, fFogDist); 

        /* 멀어질수록 1에 가까워진다. */ 
        float fFogFactor = 1.f - exp2(-pow(max(fFogDist - g_vFogParams.x, 0.f) * g_vFogParams.z, 2.f));

        /* 월드 y의 0 ~ 45 사이에 변화를 준다. */
        float fHeightMask = 1.f - smoothstep(0.f, 45.f, vWorldPos.y);
        fHeightMask = saturate(fHeightMask);
        fHeightMask *= fHeightMask;

        /* 높은 곳에서는 15% */
        fFogFactor *= lerp(0.15f, 1.f, fHeightMask);

        /* 가까운 곳도 완전 0으로는 안 떨어지게 */
        fFogFactor = lerp(0.15f, 1.f, fFogFactor);

        /* start 이전 구간은 더 약하게만 남기기 */
        fFogFactor *= lerp(0.5f, 1.f, fDistMask);

        float fNoise = frac(sin(dot(vWorldPos.xz, float2(12.9898f, 78.233f))) * 43758.5453f);
        fNoise = lerp(0.9f, 1.1f, fNoise);
        fFogFactor *= fNoise;

        fFogFactor = saturate(fFogFactor);
        fFogFactor = min(fFogFactor, 0.7f);

        vColor.rgb = lerp(vColor.rgb, g_vFogColor.rgb, fFogFactor);
    }

    Out.vColor = vColor;
    return Out;
}

PS_OUT PS_MAIN_COPY(PS_IN In)
{
    PS_OUT Out;
    Out.vColor = g_SceneTexture.Sample(DefaultSampler, In.vTexcoord);
    return Out;
}


technique11 DefaultTechnique
{
    pass Fog
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_FOG();
    }

    pass Copy
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_COPY();
    }
}