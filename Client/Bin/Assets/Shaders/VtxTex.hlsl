float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

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

float4 PS_MAIN(VS_OUT In) : SV_TARGET
{
    float4 vTextureColor = g_BaseMap.Sample(DefaultSampler, In.vTexcoord);
    return vTextureColor * g_BaseColor;
}

int g_iFrame;
int g_iRow;
int g_iCol;

float4 PS_MAIN_SPRITE_EFFECT(VS_OUT In) : SV_TARGET
{
    float2 uvCellSize = float2(1.f / g_iCol, 1.f / g_iRow);

    int iColIndex = g_iFrame % g_iCol;
    int iRowIndex = g_iFrame / g_iCol;

    float2 vUV;
    vUV.x = In.vTexcoord.x * uvCellSize.x + uvCellSize.x * iColIndex;
    vUV.y = In.vTexcoord.y * uvCellSize.y + uvCellSize.y * iRowIndex;

    float4 vTextureColor = g_BaseMap.Sample(DefaultSampler, vUV);
    return vTextureColor * g_BaseColor;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetVertexShader( CompileShader(vs_5_0, VS_MAIN()) );
        SetPixelShader(  CompileShader(ps_5_0, PS_MAIN()) );
    }

    pass SpriteEffect
{
    SetVertexShader( CompileShader( vs_5_0, VS_MAIN() ) );
    SetPixelShader( CompileShader( ps_5_0, PS_MAIN_SPRITE_EFFECT() ) );
}
}
