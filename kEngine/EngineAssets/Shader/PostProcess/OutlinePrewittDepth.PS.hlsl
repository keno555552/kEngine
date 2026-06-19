#include "FullScreenQuadVSO.hlsli"
#include "RenderCommand.hlsli"

SamplerState gSampler : register(s0);
SamplerState gDepthSampler : register(s1);

Texture2D<float4> gTexture : register(t0);
Texture2D<float> gDepthTexture : register(t2);

RenderCommand gRenderCommand : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

// Prewitt Horizontal (左右差)
static const float3x3 kPrewittH =
{
    1.0f / 6.0f, 0.0f, -1.0f / 6.0f,
    1.0f / 6.0f, 0.0f, -1.0f / 6.0f,
    1.0f / 6.0f, 0.0f, -1.0f / 6.0f
};

// Prewitt Vertical (上下差)
static const float3x3 kPrewittV =
{
    -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f,
     0.0f, 0.0f, 0.0f,
     1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f
};

float Convolve(float2 vertexcoord, float3x3 kernel)
{
    int2 size;
    gTexture.GetDimensions(size.x, size.y);
    float2 texelSize = float2(rcp((float) size.x), rcp((float) size.y));
    
    float sum = 0.0f;
    
    [unroll]
    for (int j = -1; j <= 1; ++j)
    {
        [unroll]
        for (int i = -1; i <= 1; ++i)
        {
            float2 coord = vertexcoord + float2(i, j) * texelSize;
            float depth = gDepthTexture.Sample(gDepthSampler, coord);
            sum += depth * kernel[j + 1][i + 1];
        }
    }
    return sum;
}


PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float2 uv = input.texcoord;
    float3 baseColor = gTexture.Sample(gSampler, uv).rgb;

    // Prewitt Depth Outline
    float gx = Convolve(uv, kPrewittH);
    float gy = Convolve(uv, kPrewittV);

    float edge = sqrt(gx * gx + gy * gy);
    edge *= 50.0f;
    float mask = step(gRenderCommand.depthThreshold, edge);

    //output.color.rgb = baseColor + gRenderCommand.outlineColor.rgb * edge;
    //output.color.a = 1.0f;

    output.color.rgb = lerp(baseColor, gRenderCommand.outlineColor.rgb, mask);
    output.color.a = 1.0f;

    //output.color = float4(edge.xxx, 1.0f);

    return output;
}

