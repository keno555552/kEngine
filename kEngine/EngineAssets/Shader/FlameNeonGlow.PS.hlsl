#include "CommonTypes.hlsli"

// ==============================
// Constant Buffers
// ==============================

StructuredBuffer<MaterialGroup> gMaterialList : register(t1);
ConstantBuffer<Camera> gCamera : register(b0);

// VS側の出力と一致させる
struct FlameVertexShaderOutput
{
    float4 position : SV_Position;
    float3 normal : NORMAL0;
    float3 worldPosition : POSITION0;
    float localY : TEXCOORD0;
    nointerpolation int materialId : TEXCOORD1;
};

// PS出力

PixelShaderOutput main(FlameVertexShaderOutput input)
{
    PixelShaderOutput output = {0,0,0,0};
    MaterialGroup material = gMaterialList[input.materialId];
    
    float4 finalColor = float4(0, 0, 0, 0);
    
    for (int i = 0; i < material.layerCount; ++i)
    {
        MaterialLayer layer = material.layers[i];
    // Unity:
    // float gradient = saturate(i.worldPos.y / 2.0);
    // ※Unity側の i.worldPos はローカル座標として使っていたので localY を使う
        float denom = max(layer.heightScale, 0.0001f);
        float gradient = saturate((input.localY + 1.0f) / denom);

    //gradient = saturate((input.localY + offset) / gMaterial.heightScale);

    // Unity:
    // float3 flameColor = lerp(_BaseColor.rgb, _TipColor.rgb, gradient);
        float3 flameColor = lerp(layer.color.rgb, layer.color2.rgb, gradient);

        float3 viewDir = normalize(gCamera.position - input.worldPosition);
    
        float rim = pow(1.0 - saturate(dot(normalize(input.normal), viewDir)), denom * 1.0f);
        flameColor += rim * layer.color3.rgb;

    // Unity:
    // return fixed4(flameColor * _GlowIntensity, 1.0);
    //output.color = float4(flameColor * layer.intensity, 1.0f);
        
        float3 layerColor = flameColor * layer.intensity;
        float layerAlpha = layer.color.a; // 你原本的 alpha
            
        // 正確的累積混合
        finalColor.rgb = lerp(finalColor.rgb, layerColor, layerAlpha);
        finalColor.a = max(finalColor.a, layerAlpha);
    }
    
    output.color = finalColor;
    return output;
}
