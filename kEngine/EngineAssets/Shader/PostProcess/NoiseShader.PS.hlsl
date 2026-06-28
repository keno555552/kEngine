#include "FullScreenQuadVSO.hlsli"
#include "RenderCommand.hlsli"
#include "RandomTool.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
RenderCommand gRenderCommand : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float3 base = gTexture.Sample(gSampler, input.texcoord).rgb;
    
    float n = getNoise(gRenderCommand.randomNoiseType, input.texcoord, gRenderCommand.randomNoiseTime);
    
    // 雜訊強度
    float noise = lerp(1.0, n, gRenderCommand.randomNoiseAmount);
    
    // 乘上雜訊（TV static）
    output.color.rgb = base * noise;
    output.color.a = 1.0;

    return output;
}
