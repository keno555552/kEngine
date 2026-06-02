#include "FullScreenQuadVSO.hlsli"
#include "RenderCommand.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
RenderCommand gRenderCommand : register(b0);
StructuredBuffer<BlurData> gBlurData : register(t1);

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

//static const float kKernel3x3[3][3] =
//{
//    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
//    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
//    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f }
//};


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
            float2 texcoord = vertexcoord + float2(j, i) * texelSize;
            float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
            sum += fetchColor * cb.kKernel[(j + k) * kernelSize + (i + k)];
        }
    }
    return sum;
}

float3 ConvolveBoxBlur(float2 vertexcoord,int kernelSize)
{
    int2 size;
    gTexture.GetDimensions(size.x, size.y);
    float2 texelSize = float2(rcp((float) size.x), rcp((float) size.y));
    
    float kernelNorm = 1.0f / ((float)(kernelSize * kernelSize));
    int k = kernelSize / 2;
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    
    for (int j = -k; j <= k; ++j)
    {
        for (int i = -k; i <= k; ++i)
        {
            float2 texcoord = vertexcoord + float2(j, i) * texelSize;
            sum += gTexture.Sample(gSampler, texcoord).rgb;
        }
    }
    return sum * kernelNorm;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    
    PixelShaderOutput output;
    output.color.rgb = float3(0.0f, 0.0f, 0.0f);
    output.color.a = 1.0f;

    output.color.rgb += (gRenderCommand.blurType == 0) ?
    ConvolveBoxBlur(input.texcoord, gRenderCommand.kernelSize) :
    Convolve(input.texcoord, gBlurData[0], gRenderCommand.kernelSize);
    
    return output;
}
