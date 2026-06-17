#include "FullScreenQuadVSO.hlsli"
#include "RenderCommand.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
RenderCommand gRenderCommand : register(b0);
StructuredBuffer<BlurData> gKernelData : register(t1);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float3 Convolve(float2 vertexcoord, BlurData cb, int kernelSize)
{
    int2 size;
    gTexture.GetDimensions(size.x, size.y);
    float2 texelSize = float2(rcp((float) size.x), rcp((float) size.y));

    int k = kernelSize / 2;
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    
    for (int j = -k; j <= k; ++j)
    {
        for (int i = -k; i <= k; ++i)
        {
            float2 texcoord = vertexcoord + float2(i, j) * texelSize;
            float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
            sum = sum + fetchColor * cb.kKernel[(j + k) * kernelSize + (i + k)];
        }
    }
    return sum;
}


PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color.rgb = float3(0.0f, 0.0f, 0.0f);
    output.color.a = 1.0f;
    float3 baseColor = gTexture.Sample(gSampler, input.texcoord).rgb;
    //float3 outlineColor = gRenderCommand.outlineColor;

    // Outline 專用：使用 gKernelData[1] 的 kernel
    float3 edgeColor = Convolve(input.texcoord, gKernelData[1], gRenderCommand.outlineKernelSize);
    output.color.rgb = baseColor + edgeColor  /* gRenderCommand.outlineColor*/;

    return output;
}

