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

float3 RadialBlur(float2 texcoord,float2 center,float strength,int numSamples)
{
    // 中心から現在のuvに対しての方向を計算。
    // 普段方向といえば、単位ベクトルだが、ここではあえて正規化せず、遠いほどより遠くをサンプリングする
    float2 direction = texcoord - center;
    float2 stepUV = direction * strength / numSamples;
    float3 outputColor = float3(0.0f, 0.0f, 0.0f);

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        // 現在のuvからさきほど計算した方向にサンプリング点を進めながらサンプリングしていく
        float2 sampleUV = texcoord + stepUV * sampleIndex;
        outputColor += gTexture.Sample(gSampler, sampleUV).rgb;
    }

    // 平均化する
    outputColor *= rcp(numSamples);

    return outputColor;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    
    PixelShaderOutput output;
    output.color.rgb += RadialBlur(input.texcoord/*まだ引数を渡していない*/);
    output.color.a = 1.0f;
    
    return output;
}
