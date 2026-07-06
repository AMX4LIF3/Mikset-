// ============================================================================
// ENHANCED PBR SHADER - Drop-in replacement, minimal dependencies
// ============================================================================

cbuffer SkinningBuffer : register(b3)
{
    matrix BoneMatrices[128];
};

cbuffer GlobalBuffer : register(b0)
{
    matrix ViewProj;
    matrix lightViewProj;
    float4 CameraPos;
    float4 LightDir;
};

cbuffer ObjectBuffer : register(b1)
{
    matrix World;
    matrix WVP;
    uint HasBones;
    float3 padding;
};

cbuffer MaterialBuffer : register(b2)
{
    float4 BaseColorTint;
    float MetallicFactor;
    float RoughnessFactor;
    float AOFactor;
    float EmissiveFactor;
    // padding[3] is available here if you ever want to add SubsurfaceFactor to C++
    // For now, we hardcode subsurface off or use a heuristic
};

// Original texture layout - no new slots needed
Texture2D texAlbedo : register(t0);
Texture2D texNormal : register(t1);
Texture2D texMetallicRoughness : register(t2);
Texture2D texAO : register(t3);
Texture2D texEmissive : register(t4);
SamplerState objSampler : register(s0);
TextureCube texSkybox : register(t10);

Texture2D shadowMap : register(t6);
SamplerComparisonState compSampler : register(s1);

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float4 Weight : WEIGHTS;
    uint4 BoneIds : BONEIDS;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPos : POSITION;
    float3 tangent : TANGENT;
};

static const float PI = 3.14159265359;

// ACES Filmic Tone Mapping
float3 ACESFilm(float3 x)
{
    float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// PBR functions
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

PS_INPUT VMain(VS_INPUT input)
{
    PS_INPUT output;

    float4 finalPos = float4(input.pos, 1.0f);
    float3 finalNormal = input.normal;
    float3 finalTangent = input.tangent;

    if (HasBones > 0)
    {
        float4x4 skinMat = (float4x4) 0.0f;
        float totalWeight = 0.0f;

        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            float w = input.Weight[i];
            if (w > 0.001f)
            {
                skinMat += BoneMatrices[input.BoneIds[i]] * w;
                totalWeight += w;
            }
        }

        if (totalWeight > 0.001f)
        {
            if (abs(totalWeight - 1.0f) > 0.01f)
                skinMat /= totalWeight;

            finalPos = mul(float4(input.pos, 1.0f), skinMat);
            float3x3 skinMat3 = (float3x3) skinMat;
            finalNormal = mul(input.normal, skinMat3);
            finalTangent = mul(input.tangent, skinMat3);
        }
    }

    output.pos = mul(finalPos, WVP);
    output.worldPos = mul(finalPos, World).xyz;
    output.uv = input.uv;
    output.normal = normalize(mul(finalNormal, (float3x3) World));
    output.tangent = normalize(mul(finalTangent, (float3x3) World));

    return output;
}

float4 PMain(PS_INPUT input) : SV_TARGET
{
    // --- Sampling ---
    float4 albedoTex = texAlbedo.Sample(objSampler, input.uv);
    clip(albedoTex.a - 0.1f);
    float3 albedo = pow(albedoTex.rgb, 2.2) * BaseColorTint.rgb;
    
    // Normal map with fallback
    float3 normalSample = texNormal.Sample(objSampler, input.uv).rgb;
    if (length(normalSample) < 0.01)
        normalSample = float3(0.5, 0.5, 1.0);
    
    // TBN setup
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent - dot(input.tangent, N) * N);
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);
    float3 worldNormal = normalize(mul(normalSample * 2.0 - 1.0, TBN));
    
    // Material properties
    float4 orm = texMetallicRoughness.Sample(objSampler, input.uv);
    float metallic = orm.b * MetallicFactor;
    float roughness = max(orm.g * RoughnessFactor, 0.04);
    
    if ((orm.r + orm.g + orm.b) < 0.01)
    {
        metallic = MetallicFactor;
        roughness = RoughnessFactor;
    }
    
    // AO
    float ao = orm.r;
    if (ao < 0.01)
        ao = texAO.Sample(objSampler, input.uv).r;
    if (ao < 0.01)
        ao = 1.0;
    ao *= AOFactor;
    
    // --- Direct Lighting ---
    float3 V = normalize(CameraPos.xyz - input.worldPos);
    float3 L = normalize(-LightDir.xyz); // normalize(LightPos.xyz - input.worldPos);
    float3 H = normalize(V + L);
    
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    
    float NDF = DistributionGGX(worldNormal, H, roughness);
    float G = GeometrySmith(worldNormal, V, L, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - metallic);
    float NdotL = max(dot(worldNormal, L), 0.0);
    
    float3 specular = (NDF * G * F) / (4.0 * max(dot(worldNormal, V), 0.0) * NdotL + 0.0001);
    float3 diffuse = kD * albedo / PI;
    
    // HEURISTIC: Only add warm subsurface to low-metallic, organic-looking materials
    // If metallic > 0.3 or roughness < 0.2 (shiny), skip the warm tint
    // This keeps moons/metals clean but helps skin/clay
    float organicFactor = saturate((1.0 - metallic) * (roughness - 0.1) * 2.0); // 0 to 1
    float wrapLight = pow(saturate(dot(worldNormal, L) * 0.5 + 0.5), 2.0) * 0.15;
    //diffuse += float3(1.0, 0.4, 0.2) * wrapLight * organicFactor;
    
    float3 radiance = float3(5.0, 5.0, 5.0); // Neutral white light (no warm tint)
    float3 Lo = (diffuse + specular) * radiance * NdotL;
    
    // --- Ambient (IBL) ---
    float3 R = reflect(-V, worldNormal);
    float3 envColor = texSkybox.SampleLevel(objSampler, R, roughness * 8.0).rgb;
    
    float3 F_ambient = fresnelSchlick(max(dot(worldNormal, V), 0.0), F0);
    float3 kS_ambient = F_ambient;
    float3 kD_ambient = (1.0 - kS_ambient) * (1.0 - metallic);
    
    float3 ambient = (kD_ambient * albedo * 0.5 + kS_ambient * envColor) * ao;
    
    float3 color = Lo + ambient;
    
    // --- Emissive ---
    float3 emissiveTex = texEmissive.Sample(objSampler, input.uv).rgb;
    float emissiveIntensity = dot(emissiveTex, float3(0.299, 0.587, 0.114));
    float3 emissive = (emissiveIntensity > 0.01) ? emissiveTex : albedo * 0.5;
    color += emissive * EmissiveFactor;
    
    // --- Tone Mapping & Gamma ---
    color = ACESFilm(color);
    color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
    //return float4(color, albedoTex.a);
    return float4(worldNormal * 0.5 + 0.5, 1.0);
}