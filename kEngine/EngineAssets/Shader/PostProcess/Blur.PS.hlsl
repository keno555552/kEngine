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

float3 RadialBlur(float2 vertexcoord,float2 center,float strength,int numSamples)
{
    // 中心から現在のuvに対しての方向を計算。
    // 普段方向といえば、単位ベクトルだが、ここではあえて正規化せず、遠いほどより遠くをサンプリングする
    float2 direction = vertexcoord - center;
    float2 stepUV = direction * strength / numSamples;
    float3 outputColor = float3(0.0f, 0.0f, 0.0f);
    float3 lastColor = float3(0.0f, 0.0f, 0.0f);

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        // 現在のuvからさきほど計算した方向にサンプリング点を進めながらサンプリングしていく
        float2 sampleUV = vertexcoord + stepUV * sampleIndex;
        // もしサンプリング点が画面外に出てしまう場合は、最後にサンプリングした有効な色を加算する
        if (sampleUV.x < 0.0f || sampleUV.x > 1.0f || sampleUV.y < 0.0f || sampleUV.y > 1.0f)
        {
            outputColor += lastColor;
        } else {
            /// GPUの処理の都合上、0.001くらいはみ出していることがある
            /// それてsaturateで0.001だけ内側に押し込む
            float2 safeUV = saturate(sampleUV * 0.998 + 0.001);
            outputColor += gTexture.Sample(gSampler, safeUV).rgb;
            lastColor = gTexture.Sample(gSampler, safeUV).rgb;
        }
    }

    // 平均化する
    outputColor *= rcp((float)numSamples);

    return outputColor;
}


PixelShaderOutput main(VertexShaderOutput input)
{
    
    PixelShaderOutput output;
    output.color.rgb = float3(0.0f, 0.0f, 0.0f);
    output.color.a = 1.0f;

    /// ブラーの種類によって畳み込み関数を切り替える
    switch(gRenderCommand.blurType)
    {
        case 1:
            output.color.rgb += ConvolveBoxBlur(input.texcoord, gRenderCommand.blurKernelSize);
            break;
        case 2:
            output.color.rgb += RadialBlur(input.texcoord,
                                           gRenderCommand.blurRadialCenter,
                                           gRenderCommand.blurRadialStrength,
                                           gRenderCommand.blurRadialSampleSize);
            break;
        case 3:
            output.color.rgb += Convolve(input.texcoord, gBlurData[0], gRenderCommand.blurKernelSize);
            break;
        default:
            output.color.rgb += gTexture.Sample(gSampler, input.texcoord).rgb;
            break;
    }
    return output;
}
