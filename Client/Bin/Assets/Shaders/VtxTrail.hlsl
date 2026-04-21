float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vColor    : COLOR;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vColor    : COLOR;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float4 vWorldPos = float4(In.vPosition, 1.f);
    vWorldPos = mul(vWorldPos, g_ViewMatrix);
    vWorldPos = mul(vWorldPos, g_ProjMatrix);

    Out.vPosition = vWorldPos;
    Out.vTexcoord = In.vTexcoord;
    Out.vColor = In.vColor;

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vColor    : COLOR;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In) : SV_TARGET
{
    PS_OUT Out;
    float fWhiteColor = 1 - pow(In.vTexcoord.x - 0.5f, 2) * 4.f;
    fWhiteColor = saturate(fWhiteColor);
    
    float fOuter = pow(fWhiteColor, 50.f);
    float fAlphaPow = pow(fWhiteColor, 1.2f);
    
    float3 vColor = lerp(In.vColor.rgb, float3(1.f, 1.f, 1.f), fWhiteColor);
    float fAlpha = In.vColor.a * (0.15f + fAlphaPow * 0.85f);

    Out.vColor = float4(vColor, fAlpha);
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader  = compile ps_5_0 PS_MAIN();
    }
}
