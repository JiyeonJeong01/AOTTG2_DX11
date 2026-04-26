float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4 g_ViewMatrixInverse, g_ProjMatrixInverse;
float4x4 g_LightViewMatrix;
float4x4 g_LightProjMatrix;

float g_fShadowFar = 1000.f;

texture2D g_BaseMap;
texture2D g_NormalTexture;
texture2D g_DiffuseTexture;
texture2D g_ShadeTexture;
texture2D g_SpecularTexture;
Texture2D g_DepthTexture;
Texture2D g_StaticLightDepthTexture;
Texture2D g_DynamicLightDepthTexture;

vector g_vLightDir = float4(1.f, -1.f, 1.f, 0.f);
vector g_vLightPos = float4(0.f, 0.f, 0.f, 1.f);

vector g_vDiffuseLight = float4(1.f, 1.f, 1.f, 1.f);
vector g_vAmbientLight = float4(0.35f, 0.35f, 0.35f, 1.f);
vector g_vSpecularLight = float4(1.f, 1.f, 1.f, 1.f);

vector g_vDiffuseMtrl = float4(1.f, 1.f, 1.f, 1.f);
vector g_vAmbientMtrl = float4(1.f, 1.f, 1.f, 1.f);
vector g_vSpecularMtrl = float4(1.f, 1.f, 1.f, 1.f);

vector g_vCamPosition;

float g_fLightRange = 10.f;
float fFar = 1000.f;
float g_fShininess = 50.f;

float4 g_vFogColor;
float4 g_vFogParams; // x=start, y=end, z=density, w=enable

sampler DefaultSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = wrap;
    AddressV = wrap;
};

sampler ShadowSampler = sampler_state
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

    float4x4 matWV, matWVP;

    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT_BACKBUFFER
{
    vector vColor : SV_TARGET0;
};

struct PS_OUT_LIGHT
{
    vector vShade : SV_TARGET0;
    vector vSpecular : SV_TARGET1;
};

float3 Decode_Normal(float2 vTexcoord)
{
    float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, vTexcoord);
    return normalize(vNormalDesc.xyz * 2.f - 1.f);
}

float4 Reconstruct_WorldPos(float2 vTexcoord)
{
    float4 vDepthDesc = g_DepthTexture.Sample(DefaultSampler, vTexcoord);
    float fViewZ = vDepthDesc.y * fFar;

    /* -> ndc space */
    float4 vPos;
    vPos.x = vTexcoord.x * 2.f - 1.f;
    vPos.y = vTexcoord.y * -2.f + 1.f;
    vPos.z = vDepthDesc.x;
    vPos.w = 1.f;

    /* -> clip space */
    vPos *= fViewZ;

    /* -> view space */
    vPos = mul(vPos, g_ProjMatrixInverse);
    
    /* -> world space */
    vPos = mul(vPos, g_ViewMatrixInverse);

    return vPos;
}

float Sample_StaticShadowPCF(float2 vShadowUV, float fCurLightZ, float fBias)
{
    float2 vTexelSize = float2(1.f / 8192.f, 1.f / 4608.f);

    float fResult = 0.f;

    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            float2 vUV = vShadowUV + float2(x, y) * vTexelSize;

            if (vUV.x < 0.f || vUV.x > 1.f ||
                vUV.y < 0.f || vUV.y > 1.f)
            {
                fResult += 1.f;
                continue;
            }

            float fOldStaticLightZ =
                g_StaticLightDepthTexture.Sample(ShadowSampler, vUV).y * 1000.f;

            fResult += (fOldStaticLightZ + fBias < fCurLightZ) ? 0.45f : 1.f;
        }
    }

    return fResult / 9.f;
}

float Sample_DynamicShadowPCF(float2 vShadowUV, float fCurLightZ, float fBias)
{
    float2 vTexelSize = float2(1.f / 8192.f, 1.f / 4608.f);

    float fResult = 0.f;

    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            float2 vUV = vShadowUV + float2(x, y) * vTexelSize;

            if (vUV.x < 0.f || vUV.x > 1.f ||
                vUV.y < 0.f || vUV.y > 1.f)
            {
                fResult += 1.f;
                continue;
            }

            float fOldDynamicLightZ =
                g_DynamicLightDepthTexture.Sample(ShadowSampler, vUV).y * 1000.f;

            fResult += (fOldDynamicLightZ + fBias < fCurLightZ) ? 0.55f : 1.f;
        }
    }

    return fResult / 9.f;
}

void Calc_ShadowSplit(float4 vWorldPos, float2 vScreenUV, out float fStaticShadow, out float fDynamicShadow)
{
    fStaticShadow = 1.f;
    fDynamicShadow = 1.f;

    float4 vLightPos = mul(vWorldPos, g_LightViewMatrix);
    vLightPos = mul(vLightPos, g_LightProjMatrix);

    float2 vShadowUV;
    vShadowUV.x = (vLightPos.x / vLightPos.w) * 0.5f + 0.5f;
    vShadowUV.y = (vLightPos.y / vLightPos.w) * -0.5f + 0.5f;

    if (vShadowUV.x < 0.f || vShadowUV.x > 1.f ||
        vShadowUV.y < 0.f || vShadowUV.y > 1.f)
        return;

    float3 vNormal = Decode_Normal(vScreenUV);
    float3 vLightDir = normalize(-g_vLightDir.xyz);
    float fNdotL = saturate(dot(vNormal, vLightDir));

    float fBias = lerp(0.3f, 0.1f, fNdotL);

    fStaticShadow = Sample_StaticShadowPCF(vShadowUV, vLightPos.w, fBias);
    fDynamicShadow = Sample_DynamicShadowPCF(vShadowUV, vLightPos.w, fBias);
}

PS_OUT_LIGHT PS_MAIN_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out;

    float3 vNormal = Decode_Normal(In.vTexcoord);
    float4 vWorldPos = Reconstruct_WorldPos(In.vTexcoord);

    float3 vLightDir = normalize(-g_vLightDir.xyz);
    float3 vViewDir = normalize(g_vCamPosition.xyz - vWorldPos.xyz);
    float3 vReflectDir = reflect(-vLightDir, vNormal);

    float fNdotL = max(dot(vNormal, vLightDir), 0.f);
    float fSpecular = pow(max(dot(vViewDir, vReflectDir), 0.f), g_fShininess);

    float3 vShade =
        g_vAmbientLight.rgb * g_vAmbientMtrl.rgb +
        g_vDiffuseLight.rgb * g_vDiffuseMtrl.rgb * fNdotL;

    float3 vSpec =
        g_vSpecularLight.rgb * g_vSpecularMtrl.rgb * fSpecular;

    Out.vShade = float4(vShade, 1.f);
    Out.vSpecular = float4(vSpec, 1.f);

    return Out;
}

PS_OUT_LIGHT PS_MAIN_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out;

    float3 vNormal = Decode_Normal(In.vTexcoord);
    float4 vWorldPos = Reconstruct_WorldPos(In.vTexcoord);

    float3 vToLight = g_vLightPos.xyz - vWorldPos.xyz;
    float fDistance = length(vToLight);
    float3 vLightDir = normalize(vToLight);

    float fAtt = saturate((g_fLightRange - fDistance) / g_fLightRange);

    float3 vViewDir = normalize(g_vCamPosition.xyz - vWorldPos.xyz);
    float3 vReflectDir = reflect(-vLightDir, vNormal);

    float fNdotL = max(dot(vNormal, vLightDir), 0.f);
    float fSpecular = pow(max(dot(vViewDir, vReflectDir), 0.f), g_fShininess);

    float3 vShade =
        (g_vAmbientLight.rgb * g_vAmbientMtrl.rgb +
         g_vDiffuseLight.rgb * g_vDiffuseMtrl.rgb * fNdotL) * fAtt;

    float3 vSpec =
        (g_vSpecularLight.rgb * g_vSpecularMtrl.rgb * fSpecular) * fAtt;

    Out.vShade = float4(vShade, 1.f);
    Out.vSpecular = float4(vSpec, 1.f);

    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_COMBINED(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;

    float4 vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vDiffuse.a == 0.f)
        discard;

    float4 vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vSpecular = g_SpecularTexture.Sample(DefaultSampler, In.vTexcoord);

    float4 vColor = float4(vDiffuse.rgb * vShade.rgb + vSpecular.rgb, vDiffuse.a);

    float4 vWorldPos = Reconstruct_WorldPos(In.vTexcoord);

    float fStaticShadow = 1.f;
    float fDynamicShadow = 1.f;

    Calc_ShadowSplit(vWorldPos, In.vTexcoord, fStaticShadow, fDynamicShadow);

    /* Static shadow는 벽/건물/바닥 전부 받음 */
    vColor.rgb *= fStaticShadow;

    /* Dynamic shadow는 바닥 근처에서만 받음 */
    float fGroundY = 1.f;
    float fGroundMask = 1.f - smoothstep(0.5f, 1.5f, abs(vWorldPos.y - fGroundY));

    vColor.rgb *= lerp(1.f, fDynamicShadow, fGroundMask);

    // if (g_vFogParams.w > 0.5f) // x=start, y=end, z=density, w=enable
    // {
    //     float fFogDist = length(g_vCamPosition.xyz - vWorldPos.xyz);

    //     /* start~end 구간 마스크 */
    //     float fDistMask = smoothstep(g_vFogParams.x, g_vFogParams.y, fFogDist);

    //     /* exp2 fog */
    //     float fFogFactor = 1.f - exp2(-pow(fFogDist * g_vFogParams.z, 2.f));

    //     /* 낮은 높이에서만 안개, 너무 빡세지 않게 */
    //     float fHeightMask = 1.f - smoothstep(5.f, 25.f, vWorldPos.y);

    //     fFogFactor *= fDistMask;
    //     fFogFactor *= fHeightMask;

    //     /* 너무 하얗게 덮이지 않게 최대치 제한 */
    //     fFogFactor = saturate(fFogFactor);
    //     fFogFactor = min(fFogFactor, 0.65f);

    //     vColor.rgb = lerp(vColor.rgb, g_vFogColor.rgb, fFogFactor);
    // }

    if (g_vFogParams.w > 0.5f)
{
    float3 vFogVec = g_vCamPosition.xyz - vWorldPos.xyz;
    float fFogDistSq = dot(vFogVec, vFogVec);

    float fStart = g_vFogParams.x;
    float fEnd = g_vFogParams.y;

    float fStartSq = fStart * fStart;
    float fEndSq = fEnd * fEnd;

    float fFogFactor = saturate((fFogDistSq - fStartSq) / max(fEndSq - fStartSq, 0.0001f));

    float fHeightMask = saturate((25.f - vWorldPos.y) / 20.f);

    fFogFactor *= fHeightMask;
    fFogFactor = min(fFogFactor, 0.5f);

    vColor.rgb = lerp(vColor.rgb, g_vFogColor.rgb, fFogFactor);
}

    Out.vColor = vColor;
    return Out;
}


technique11 DefaultTechnique
{
    pass Directional
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL();
    }

    pass Point
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_POINT();
    }

    pass Combined
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_COMBINED();
    }
}