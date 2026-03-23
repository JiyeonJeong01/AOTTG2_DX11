float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

vector g_vLightDir = vector (1.f, -1.f, 1.f, 0.f);
vector g_vLightDiffuse = vector(1.f, 1.f, 1.f, 1.f) ;
vector g_vLightAmbient = vector(1.f, 1.f, 1.f, 1.f);
vector g_vLightSpecular = vector(1.f, 1.f, 1.f, 1.f);

texture2D g_BaseMap;
vector g_vMtrlAmbient = vector(0.3f, 0.3f, 0.3f, 1.f);
vector g_vMtrlSpecular = vector(1.f, 1.f, 1.f, 1.f);
vector g_vCamPosition = vector(1.f, 1.f, 1.f, 1.f);

sampler DefaultSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = wrap;
    AddressV = wrap;
};

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float4x4 matWV, matWVP;
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);

    Out.vNormal = mul(float4(In.vNormal, 1.f), g_WorldMatrix);

    Out.vTexcoord = In.vTexcoord * 70;

    Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
};

struct PS_OUT
{
    vector vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;

    float4 vLightDir = normalize(g_vLightDir);
    vector vDiffuseColor = g_BaseMap.Sample(DefaultSampler, In.vTexcoord);
    vector vShade = saturate(saturate(dot(-vLightDir, In.vNormal)) + (g_vLightAmbient * g_vMtrlAmbient));

    vector vLook = normalize(In.vWorldPos - g_vCamPosition);
    vector vReflect = normalize(reflect(vLightDir, In.vNormal));
    float fSpecular = pow(saturate(dot(vLook * -1.f, vReflect)), 50.f);

    vector vSpecularColor = g_vLightSpecular * g_vMtrlSpecular * fSpecular;
    Out.vColor = g_vLightDiffuse * vDiffuseColor * vShade + vSpecularColor;
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass TerrainPass
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
