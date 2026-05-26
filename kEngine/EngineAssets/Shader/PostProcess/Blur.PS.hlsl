#include "FullScreenQuadVSO.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

static const float2 kIndex3x3[3][3] =
{
    { float2(-1.0f, -1.0f), float2(0.0f, -1.0f), float2(1.0f, -1.0f) },
    { float2(-1.0f, 0.0f), float2(0.0f, 0.0f), float2(1.0f, 0.0f) },
    { float2(-1.0f, 1.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f) }
};

static const float kKernel3x3[3][3] =
{
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f }
};


//float3 Convolve3x3(float2 uv, float2 step, float kernel[3][3])
//{
//    float3 sum = 0;
//    
//    for (int y = -1; y <= 1; ++y)
//        
//        for (int x = -1; x <= 1; ++x)
//            
//            sum += gTexture.Sample(gSampler, uv + float2(x, y) * step).rgb * kernel[y + 1][x + 1];
//    
//    return sum;
//    
//}



PixelShaderOutput main(VertexShaderOutput input)
{
    int width, height;
    gTexture.GetDimensions(width, height);
    float2 uvStepSize = float2(rcp((float) width), rcp((float) height));
    
    PixelShaderOutput output;
    output.color.rgb = float3(0.0f, 0.0f, 0.0f);
    output.color.a = 1.0f;
    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            float2 texcoord = input.texcoord + kIndex3x3[y][x] * uvStepSize;
            
            float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
            
            output.color.rgb += fetchColor * kKernel3x3[y][x];
        }
    }
    
    return output;
}
