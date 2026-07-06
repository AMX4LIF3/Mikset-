cbuffer SkinningBuffer : register(b3)
{
    matrix BoneMatrices[128];
};

cbuffer ObjectBuffer : register(b1)
{
    matrix World;
    matrix WVP;
    int HasBones;
};
/*
struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float4 tangent : TANGENT;
};
*/


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

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
};

VS_OUTPUT VMain(VS_INPUT input)
{
    
    VS_OUTPUT o;
  /*  
    //o.pos = mul(float4(input.pos, 1.0f), WVP);
    o.pos = mul(float4(input.pos, 1.0f), WVP);
    //o.pos = mul(World, (float4(input.pos, 1.0f)));

    return o;
*/
    

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

       o.pos = mul(finalPos, WVP);
 //   output.worldPos = mul(finalPos, World).xyz;
   // output.uv = input.uv;
 //   output.normal = normalize(mul(finalNormal, (float3x3) World));
    //output.tangent = normalize(mul(finalTangent, (float3x3) World));
    return o;
}