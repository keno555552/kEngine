#include "CommonTypes.hlsli"
#include "SkyCube.hlsli"

// Resources
SamplerState gSampler : register(s0);
TextureCube<float4> gTexture : register(t0);
StructuredBuffer<MaterialGroup> gMaterialList : register(t1);

PixelShaderOutput main(SkyCubeVSOutput input)
{
    PixelShaderOutput output;
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    output.color = textureColor * gMaterialList[input.materialId].layers[0].color;
    return output;
}
