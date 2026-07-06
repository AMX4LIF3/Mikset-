cbuffer SkyboxBuffer : register(b10)
{
    matrix viewProj;
};

TextureCube skybox : register(t0);
SamplerState skySampler : register(s0);

struct VS_INPUT
{
    float3 pos : POSITION;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 texCoord : TEXCOORD;
};

PS_INPUT VMain(VS_INPUT input)
{
    PS_INPUT output;
    
    float4 projectedPos = mul(float4(input.pos, 1.0f), viewProj);
    
    // setting z = w so depth is always 1.0 
    output.pos = projectedPos.xyww;
    
    output.texCoord = input.pos;
    
    return output;
}

float4 PMain(PS_INPUT input) : SV_Target
{
    float4 color = skybox.Sample(skySampler, input.texCoord);
    
    // apply a tiny bit of exposure so it doesn't look flat or dark
    float exposure = 1.2f;
    return float4(color.rgb * exposure, 1.0f);
}