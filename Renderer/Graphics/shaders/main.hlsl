/*
  \/\/\/\/\/\/\/\/\/\/\/\/\/
|=============================|
|     Mikset Standard PBR     |
|                             |
|                             |  

*/      

cbuffer GlobalBuffer : register(b0)
{
    matrix ViewProj;
    matrix LightViewProj;
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
    // should probably implement SubSurfaceFactor here aswell
};

cbuffer SkinningBuffer : register(b3)
{
    matrix BoneMatrices[128];
};

Texture2D texAlbedo : register(t0);
Texture2D texNormal : register(t1);
Texture2D texMetallicRoughness : register(t2);
Texture2D texAO : register(t3);
Texture2D texEmissive : register(t4);

TextureCube texSkybox : register(t10);

SamplerState objSampler : register(s0);

Texture2D shadowMap : register(t6);
SamplerState shadowSampler : register(s1);
SamplerComparisonState compSampler : register(s2);

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
    float4 shadowpos : TEXCOORD1;
};
static const float PI = 3.14159265359;

float3 ACESFilm(float3 x)
{

    float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;

    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));

}


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



float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness) // i like 'em rough bawbeh
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) *
           pow(1.0 - cosTheta, 5.0);

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
    
    output.shadowpos = mul(float4(output.worldPos, 1.0), LightViewProj); // project to light space
    //output.shadowpos = -output.shadowpos;
    
    return output;
}



float4 PMain(PS_INPUT input) : SV_TARGET
{
    
    // materials 
    float4 albedoTex = texAlbedo.Sample(objSampler, input.uv);

    clip(albedoTex.a - 0.1f);

    float3 albedo = pow(albedoTex.rgb, 2.2) * BaseColorTint.rgb;


    // normals
    float3 normalSample = texNormal.Sample(objSampler, input.uv).rgb;

    if (length(normalSample) < 0.01f)
    {
        normalSample = float3(0.5f, 0.5f, 1.0f);
    }

    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent - dot(input.tangent, N) * N);
    float3 B = normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);

    float3 worldNormal = normalize(mul(normalSample * 2.0f - 1.0f, TBN));

    
    float4 orm = texMetallicRoughness.Sample(objSampler, input.uv);
    float metallic = orm.b * MetallicFactor;
    float roughness = clamp(orm.g * RoughnessFactor, 0.04f, 1.0f);

    if ((orm.r + orm.g + orm.b) < 0.01f)
    {
        metallic = MetallicFactor;
        roughness = RoughnessFactor;
    }

    float ao = orm.r;

    if (ao < 0.01f || ao > 0.99f)
    {
        ao = texAO.Sample(objSampler, input.uv).r;

        if (ao < 0.01f)
        {
            ao = 1.0f;
        }
    }

    ao *= AOFactor;

    //lighting
    float3 V = normalize(CameraPos.xyz - input.worldPos);

    // negated the lighting direction because thats the vector its traveling TO.
    // idk how to explain but say the light starts at pos 0, when you set it to say -10
    // it means the light will travel TO that pos. thus the lighitng appears coming from the sky 
    float3 L = normalize(-LightDir.xyz);
    float3 H = normalize(V + L);
    float NdotL = max(dot(worldNormal, L), 0.0f);


    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);
    float NDF = DistributionGGX(worldNormal, H, roughness);
    float G = GeometrySmith(worldNormal, V, L, roughness);
    //float3 F = fresnelSchlickRoughness(max(dot(H, V), 0.0f), F0, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);

    float3 kS = F;
    float3 kD = (1.0f - kS) * (1.0f - metallic);

    float3 numerator = NDF * G * F;
    float denominator = 4.0f * max(dot(worldNormal, V), 0.0f) * NdotL + 0.0001f;

    float3 specular = numerator / denominator;

    // shadows
    // light clip space
    float3 shadowNDC = input.shadowpos.xyz / input.shadowpos.w;

    float2 shadowUV;
    shadowUV.x = shadowNDC.x * 0.5f + 0.5f;
    shadowUV.y = -shadowNDC.y * 0.5f + 0.5f;
    
    // wow this actually worked
    bool outsideShadowMap =
        shadowUV.x < 0.0f ||
        shadowUV.x > 1.0f ||
        shadowUV.y < 0.0f ||
        shadowUV.y > 1.0f ||
        shadowNDC.z < 0.0f ||
        shadowNDC.z > 1.0f;

    // instead of a massive depth bias i shift the position slightly along the world normal
    // this should prevent shadow acne on thin structures without pushing the depth through the mesh
    float shadowTexelSize = 1.0f / 1024.0f;
    float3 biasedWorldPos = input.worldPos + worldNormal * (shadowTexelSize * 2.0f);
    
    float bias = max(0.0005f * (1.0f - NdotL), 0.0001f); // Lowered by a factor of 5!
    float currentDepth = shadowNDC.z - bias;
    
    // sample shadow map
    float shadowDepth = shadowMap.Sample(shadowSampler, shadowUV).r;
    float shadowFactor = 1.0f;
    
    // thanks random guy on an old ass forum for this
    if (!outsideShadowMap)
    {
        float sum = 0.0f;
        float2 texelSize = float2(1.0f / 1024.0f, 1.0f / 1024.0f);
        
        float blurRadius = 0.5f;

        // Calculate a fast pseudo-random angle based on screen position
        float3 magic = float3(0.06711056f, 0.00583715f, 52.9829189f);
        float noise = frac(magic.z * frac(dot(input.pos.xy, magic.xy))); // Interleaved Gradient Noise
        
        // Create a fast 2D rotation matrix from the noise angle
        float s = sin(noise * 6.283185f);
        float c = cos(noise * 6.283185f);
        float2x2 rotationMatrix = float2x2(c, -s, s, c);

        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                // Standard offset
                float2 offset = float2(x, y) * texelSize * blurRadius;
                
                // Rotate the offset using our screen noise matrix
                offset = mul(offset, rotationMatrix);
                
                float2 OffsetUV = shadowUV + offset;
                
                float shadowDepth = shadowMap.Sample(shadowSampler, OffsetUV).r;
                sum += (currentDepth <= shadowDepth) ? 2.0f : 0.0f;
            }
        }
        
        shadowFactor = sum / 9.0f;
        shadowFactor = max(shadowFactor, 0.15f);
    }
    
    
    float3 diffusePart = kD * albedo / PI;
    float3 radiance = float3(4.0f, 4.0f, 4.0f); // nuetral kinda like white light
    float3 direct = (diffusePart + specular) * radiance *  NdotL;

    direct *= shadowFactor;

    
    // fake reflictions
    float3 R = reflect(-V, worldNormal);

    float3 envColor = texSkybox.SampleLevel(objSampler, R, roughness * 8.0f).rgb;

    float3 F_ambient = fresnelSchlickRoughness(max(dot(worldNormal, V), 0.0f), F0, roughness);
    float3 kS_ambient = F_ambient;
    float3 kD_ambient =  (1.0f - kS_ambient) *  (1.0f - metallic);
    float3 diffuseAmbient = kD_ambient *  albedo * envColor *  0.2f;
    float3 specularAmbient = kS_ambient * envColor;
    float3 ambient = (diffuseAmbient + specularAmbient) * ao;

    float3 color = direct + ambient;
/*
    float3 emissiveTex = texEmissive.Sample(objSampler, input.uv).rgb;

    float emissiveIntensity =
        dot(emissiveTex, float3(0.299f, 0.587f, 0.114f));

    float3 emissive = (emissiveIntensity > 0.01f) ? emissiveTex : float3(0, 0, 0);

    color += emissive * EmissiveFactor;
*/
float3 emissive = texEmissive.Sample(objSampler, input.uv).rgb;

// 2. Add it directly to the accumulated color, scaled by your material constant multiplier
color += emissive * EmissiveFactor;
   
// honestly looks bad keep it off
#define FOG_DENSITY 0 // 0.0005f
#define FOG_COLOR float3(0.3f, 0.2f, 0.0f)

    float fogDist = length(CameraPos.xyz - input.worldPos);
    float fogFactor = exp(-fogDist * FOG_DENSITY);

    color = lerp(FOG_COLOR, color, fogFactor);

    // tonemap
    color = ACESFilm(color);

    color = pow(color, 1.0f / 2.2f);

    return float4(color, albedoTex.a);
}
