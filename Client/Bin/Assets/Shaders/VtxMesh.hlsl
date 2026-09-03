/* constant table */
float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_BaseMap;
vector g_BaseColor = vector(1.f, 1.f, 1.f, 1.f);


float g_OutlineWidth = 0.05f;
vector g_OutlineColor = (1.f, 1.f, 1.f, 1.f);

float g_fFar = 1000.f;

Texture2D   g_DissolveNoiseMap;

float       g_DissolveAmount = 0.f;
float       g_DissolveEdgeWidth = 0.05f;
vector      g_DissolveEdgeColor = vector(1.f, 0.35f, 0.05f, 1.f);

SamplerState DefaultSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState DissolveSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

struct PS_OUT
{
    vector vColor : SV_TARGET0;
    vector vNormal : SV_TARGET1;
    vector vDepth : SV_TARGET2;
};

VS_OUT VS_Default(VS_IN In)
{
    VS_OUT Out;
    float4x4 matWV, matWVP;

    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vProjPos = Out.vPosition;

    return Out;
}

PS_OUT PS_Default(PS_IN In)
{
    PS_OUT Out;
    float4 vTextureColor = g_BaseMap.Sample(DefaultSampler, In.vTexcoord);

    if (vTextureColor.a < 0.3f)
        discard;

    Out.vColor = vTextureColor * g_BaseColor;
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.f, 1.f);

    return Out;
}

VS_OUT VS_Outline(VS_IN In)
{
    VS_OUT Out;
    float4x4 matWV, matWVP;

    float3 vNormal = normalize(In.vNormal);
    float3 vOutlinePos = In.vPosition + vNormal * g_OutlineWidth;

    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(float4(vOutlinePos, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    Out.vWorldPos = mul(float4(vOutlinePos, 1.f), g_WorldMatrix);
    Out.vProjPos = Out.vPosition;

    return Out;
}

PS_OUT PS_Outline(PS_IN In)
{
    PS_OUT Out;

    Out.vColor = g_OutlineColor;
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.f, 1.f);

    return Out;
}

struct PS_OUT_SHADOW
{
    vector vLightDepth : SV_TARGET0;
};

PS_OUT_SHADOW PS_Shadow(PS_IN In)
{
    PS_OUT_SHADOW Out;

    Out.vLightDepth = vector(
        In.vProjPos.z / In.vProjPos.w,
        In.vProjPos.w / 1000.f,
        0.f,
        1.f);
    return Out;
}

// PS_OUT PS_Dissolve(PS_IN In)
// {
//     PS_OUT Out;

//     vector vMtrlDiffuse = g_BaseMap.Sample(DefaultSampler, In.vTexcoord);

//     if (vMtrlDiffuse.a < 0.3f)
//         discard;

//     float fNoise = g_DissolveNoiseMap.Sample(DessolveSampler, In.vTexcoord).r;

//     clip(fNoise - g_DissolveAmount);

//     float fEdge = 1.f - saturate((fNoise - g_DissolveAmount) / max(g_DissolveEdgeWidth, 0.0001f));

//     vector vColor = vMtrlDiffuse * g_BaseColor;
//     vColor.rgb = lerp(vColor.rgb, g_DissolveEdgeColor.rgb, fEdge);

//     Out.vColor = vColor;
//     Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
//     Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.f, 1.f);

//     return Out;
// }

PS_OUT PS_Dissolve(PS_IN In)
{
    PS_OUT Out;

    vector vMtrlDiffuse = g_BaseMap.Sample(DefaultSampler, In.vTexcoord);

    if (vMtrlDiffuse.a < 0.3f)
        discard;

    float fNoise = g_DissolveNoiseMap.Sample(DissolveSampler, In.vTexcoord).r;

    clip(fNoise - g_DissolveAmount);

    float fEdge = 1.f - saturate((fNoise - g_DissolveAmount) / max(g_DissolveEdgeWidth, 0.0001f));

    vector vColor = vMtrlDiffuse * g_BaseColor;
    vColor.rgb = lerp(vColor.rgb, g_DissolveEdgeColor.rgb, fEdge);

    Out.vColor = vColor;
    Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.f, 1.f);

    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetVertexShader(CompileShader(vs_5_0, VS_Default()));
        SetPixelShader(CompileShader(ps_5_0, PS_Default()));
    }

    pass OutlinePass
    {
        SetVertexShader(CompileShader(vs_5_0, VS_Outline()));
        SetPixelShader(CompileShader(ps_5_0, PS_Outline()));
    }

    pass ShadowPass
    {
        SetVertexShader(CompileShader(vs_5_0, VS_Default()));
        SetPixelShader(CompileShader(ps_5_0, PS_Shadow()));
    }
        pass Dissolve
    {
        SetVertexShader(CompileShader(vs_5_0, VS_Default()));
        SetPixelShader(CompileShader(ps_5_0, PS_Dissolve()));
    }
}
