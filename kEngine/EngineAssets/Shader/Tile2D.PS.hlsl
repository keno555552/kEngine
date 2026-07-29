#include "CommonTypes.hlsli"

StructuredBuffer<MaterialGroup> gMaterialList : register(t1);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output = { 0, 0, 0, 0 };
    int usingLayer = input.materialId;

    for (int i = 0; i < gMaterialList[usingLayer].layerCount; i++)
    {
        MaterialLayer usinglayer = gMaterialList[usingLayer].layers[i];
        
        float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), usinglayer.uvTransform);
        float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
        
        if (textureColor.a <= 0.5 || textureColor.a == 0.0)
            continue;
        
        
        float3 layerColor = usinglayer.color.rgb * textureColor.rgb;
        output.color.rgb = lerp(output.color.rgb, layerColor, textureColor.a);
        output.color.a = max(output.color.a, textureColor.a);

    }
    if (output.color.a == 0.0) return float4(0, 0, 0, 0);
    
    return output;
}

