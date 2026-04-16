float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D   g_BaseMap;
vector      g_BaseColor = ( 1.f, 1.f, 1.f, 1.f );

matrix g_BoneMatrices[512];

float g_OutlineWidth;
vector g_OutlineColor;

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
    float3 vTangent : TANGENT;
    float2 vTexcoord : TEXCOORD0;    
    uint4 vBlendIndex : BLENDINDEX;
    float4 vBlendWeight : BLENDWEIGHT;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
};

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

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    float4x4 matWV, matWVP;
    
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    
    matrix BoneMatrix = g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x + 
        g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y + 
        g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z + 
        g_BoneMatrices[In.vBlendIndex.w] * fWeightW;
    
    vector vPosition = mul(float4(In.vPosition, 1.f), BoneMatrix);
    
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(vPosition, matWVP);
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    
    return Out;
}

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    
    vector      vMtrlDiffuse = g_BaseMap.Sample(DefaultSampler, In.vTexcoord);
    
    if (vMtrlDiffuse.a < 0.3f)
        discard;

    Out.vColor = vMtrlDiffuse * g_BaseColor;
    
    return Out;
}

VS_OUT VS_OUTLINE(VS_IN In)
{
    VS_OUT Out;
    
    float4x4 matWV, matWVP;

    float3 vNormal = normalize(In.vNormal);
    float3 vOulinePos = In.vPosition + vNormal * g_OutlineWidth;
    
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    
    matrix BoneMatrix = g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x + 
        g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y + 
        g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z + 
        g_BoneMatrices[In.vBlendIndex.w] * fWeightW;
    
    vector vPosition = mul(float4(In.vPosition, 1.f), BoneMatrix);
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(float4(vOulinePos, 1.f), matWVP);    
    return Out;
}


PS_OUT PS_OUTLINE(PS_IN In)
{
    PS_OUT Out;
    Out.vColor = g_OutlineColor;
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass Outline
    {
        VertexShader = compile vs_5_0 VS_OUTLINE();
        PixelShader  = compile ps_5_0 PS_OUTLINE();
    }
}
