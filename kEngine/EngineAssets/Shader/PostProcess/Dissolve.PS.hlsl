#include "FullScreenQuadVSO.hlsli"
#include "RenderCommand.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
RenderCommand gRenderCommand : register(b0);
StructuredBuffer<BlurData> gBlurData : register(t1);
Texture2D<float4> gMaskTexture : register(t3);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
	PixelShaderOutput output;
    float mask = gMaskTexture.Sample(gSampler, input.texcoord).r;
    float edge = 1.0f - smoothstep(gRenderCommand.dissolveThreshold, gRenderCommand.dissolveThreshold + gRenderCommand.dissolveEdgeWidth, mask);

	/// DissolveThresholdより小さい場合はピクセルを破棄する
	if (mask < gRenderCommand.dissolveThreshold){
		discard;
	}
	
	/// Edgeが一定以上の値を持つ場合はピクセルを破棄する
	output.color = gTexture.Sample(gSampler, input.texcoord);
	output.color.rgb += edge * gRenderCommand.dissolveEdgeColor; // Edge部分をオレンジ色に強調表示（例）
    return output;
}

