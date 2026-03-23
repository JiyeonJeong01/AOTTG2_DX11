/* constant table */
float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_BaseMap;
vector g_BaseColor;

SamplerState DefaultSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
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
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    float4x4 matWV, matWVP;
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;

    return Out;
}

float4 PS_MAIN(VS_OUT In) : SV_TARGET
{
    float4 vTextureColor = g_BaseMap.Sample(DefaultSampler, In.vTexcoord);

    if (vTextureColor.a < 0.3f)
        discard;

    return vTextureColor * g_BaseColor;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetVertexShader( CompileShader(vs_5_0, VS_MAIN()) );
        SetPixelShader(  CompileShader(ps_5_0, PS_MAIN()) );
    }
}
