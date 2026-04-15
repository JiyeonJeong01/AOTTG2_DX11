/* constant table */
float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_BaseMap;
vector g_BaseColor;

float g_OutlineWidth = 0.05f;
vector g_OutlineColor = (1.f, 1.f, 1.f, 1.f);

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

VS_OUT VS_Default(VS_IN In)
{
    VS_OUT Out;
    float4x4 matWV, matWVP;
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;

    return Out;
}

float4 PS_Default(VS_OUT In) : SV_TARGET
{
    float4 vTextureColor = g_BaseMap.Sample(DefaultSampler, In.vTexcoord);

    if (vTextureColor.a < 0.3f)
        discard;

    return vTextureColor * g_BaseColor;
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

    return Out;
}

float4 PS_Outline(VS_OUT In) : SV_TARGET
{
    return g_OutlineColor;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetVertexShader( CompileShader(vs_5_0, VS_Default()) );
        SetPixelShader(  CompileShader(ps_5_0, PS_Default()) );
    }

    pass OutlinePass
    {
        SetVertexShader( CompileShader(vs_5_0, VS_Outline()) );
        SetPixelShader(  CompileShader(ps_5_0, PS_Outline()) ); 
    }
}
