float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

TextureCube g_BaseMap;
vector g_BaseColor = (1.f, 1.f, 1.f, 1.f);

SamplerState DefaultSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float3 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float4 p = float4(In.vPosition, 1.0f);

    float4x4 matView = g_ViewMatrix;
    matView._41 = 0.0f;
    matView._42 = 0.0f;
    matView._43 = 0.0f;

    p = mul(p, g_WorldMatrix);
    p = mul(p, matView);
    p = mul(p, g_ProjMatrix);

    Out.vPosition = p;
    Out.vPosition.z = Out.vPosition.w;

    Out.vTexcoord = normalize(In.vTexcoord);
    return Out;
}

float4 PS_MAIN(VS_OUT In) : SV_TARGET
{
    float4 vTextureColor = g_BaseMap.Sample(DefaultSampler, normalize(In.vTexcoord));
    return vTextureColor;
    //return vTextureColor * g_BaseColor;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetVertexShader( CompileShader(vs_5_0, VS_MAIN()) );
        SetPixelShader(  CompileShader(ps_5_0, PS_MAIN()) );
    }
}
