#include "CommonTypes.hlsli"
#if defined(LIGHT_MODEL_LAMBERT)
#include "./LightingLambert/Lambert.hlsl"
#endif

#if defined(LIGHT_MODEL_HALF)
#include "./LightingLambert/HalfLambert.hlsl"
#endif

#if defined(LIGHT_MODEL_PHONG)
#include "./LightingLambert/PhongReflection.hlsl"
#endif

#if defined(LIGHT_MODEL_BLINN_PHONG)
#include "./LightingLambert/BlinnPhongReflection.hlsl"
#endif


// Constant buffers
ConstantBuffer<Camera> gCamera : register(b0);
cbuffer LightCountCB : register(b1)
{
    uint gLightCount;
};

// Resources
Texture2D<float4> gTexture : register(t0);
StructuredBuffer<MaterialGroup> gMaterialList : register(t1);
StructuredBuffer<LightGPU> gLights : register(t2);

// 環形マップ
TextureCube<float4> gEnvironmentTexture : register(t3);

// Resources
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = float4(0, 0, 0, 0);
    
    /// MaterialGroupを取得
    int usingLayer = input.materialId;
    MaterialGroup usingMaterialGroup = gMaterialList[usingLayer];
    
    float3 layerColor = 0;
    float layerAlpha = 0;
    
    /// レイヤーごとに処理
    for (int i = 0; i < usingMaterialGroup.layerCount; i++)
    {
        MaterialLayer usinglayer = usingMaterialGroup.layers[i];
        
        float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), usinglayer.uvTransform);
        float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
        if (usinglayer.enableLighting == 1)
        {
            float3 result = 0;
            float totalWeight = 0.0f;

            float3 N = normalize(input.normal);

            uint lightCount = gLightCount;
        
        // 多光源迴圈
        [loop]
            for (uint i = 0; i < lightCount; i++)
            {
                LightGPU L = gLights[i];
            
                float3 lightDir = 0;
                float3 lightColor = 0;
                bool validLight = true;
            
            // DirectionalLight
                if (L.type == 0)
                {
                    lightDir = normalize(-L.direction);
                    lightColor = L.color * L.intensity;
                }
            
            // Point Light
                else if (L.type == 1)
                {
                    float3 toLight = L.position - input.worldPosition;
                    float dist = length(toLight);
                    lightDir = normalize(toLight);

                // 距離衰減
                    float atten = saturate(1.0f - dist / L.range);

                    lightColor = L.color * L.intensity * atten;
                }
            
            // Spot Light
                else if (L.type == 2)
                {
                    float3 toLight = L.position - input.worldPosition;
                    float dist = length(toLight);
                    lightDir = normalize(toLight);

                // 距離衰減
                    float atten = saturate(1.0f - dist / L.range);

            // Spot cone（方向衰減）
                    float cosTheta = dot(lightDir, normalize(-L.direction));
                    float spotFactor = saturate(cosTheta / cos(L.angle));

                    lightColor = L.color * L.intensity * atten * spotFactor;
                }
                else
                {
                    validLight = false;
                }

                if (!validLight)
                    continue;

            
            
#if defined(LIGHT_MODEL_LAMBERT)
                result += ApplyLighting_Lambert(N, lightDir, lightColor);
                totalWeight += 1.0f;
#endif
        
#if defined(LIGHT_MODEL_HALF)
                result += ApplyLighting_HalfLambert(N, lightDir, lightColor);
                totalWeight += 1.0f;
#endif
       
#if defined (LIGHT_MODEL_PHONG)

                result += ApplyLighting_PhongReflection(N,
                                                        lightDir,
                                                        lightColor,
                                                        input.worldPosition,
                                                        gCamera.position,
                                                        usinglayer.shininess);
                totalWeight += 1.0f;
#endif 
       
#if defined (LIGHT_MODEL_BLINN_PHONG)

                result += ApplyLighting_BlinnPhongReflection(N,
                                                            lightDir,
                                                            lightColor,
                                                            input.worldPosition,
                                                            gCamera.position,
                                                            usinglayer.shininess);
                totalWeight += 1.0f;
#endif 
       
            
        //result += gMaterial.color.rgb * textureColor.rgb;
        //totalWeight += 1.0f;
        
        
        //lightFactor = (gMaterial.color.rgb + lightFactor) * 0.5f;
        //float3 finalColor = textureColor.rgb * lightFactor * gDirectionalLight.intensity;
        
            }
            
            float3 lightFactor = result / max(totalWeight, 1.0f);
            layerColor = usinglayer.color.rgb * textureColor.rgb * lightFactor;
            layerAlpha = usinglayer.color.a * textureColor.a;
        
#if defined(USE_ENVIRONMENT_REFLECTION)
            float3 cameraToPosition = normalize(input.worldPosition - gCamera.position);
            float3 reflectedVector = normalize(reflect(cameraToPosition, normalize(input.normal)));
            float3 environmentColor = gEnvironmentTexture.Sample(gSampler, reflectedVector).rgb;
 
            // 混合
            layerColor += environmentColor * usinglayer.reflectiveStrength;
#endif
            
            //output.color.rgb = lerp(output.color.rgb, layerColor, textureColor.a);
            //output.color.a = max(output.color.a, layerAlpha);
            output.color.rgb = output.color.rgb * (1.0f - layerAlpha) + layerColor * layerAlpha;
            //output.color.rgb = float3(0.0f,1.0f,0.0f);
            output.color.a = layerAlpha + output.color.a * (1.0f - layerAlpha);
            //output.color.a = 0.8f;
        }
        else
        {
            layerColor = usinglayer.color.rgb * textureColor.rgb;
            layerAlpha = usinglayer.color.a * textureColor.a;
            
            output.color.rgb = output.color.rgb * (1.0f - layerAlpha) + layerColor * layerAlpha;
            //output.color.rgb = float3(1.0f,0.0f,0.0f);
            output.color.a = layerAlpha + output.color.a * (1.0f - layerAlpha);
            //output.color.rgb = usinglayer.color.rgb * textureColor.rgb;
            //output.color.a = usinglayer.color.a * textureColor.a;
        }
    //output.color.rgb = normalize(input.normal) * 0.5f + 0.5f;
    //output.color.a = 1.0f;
    }
    return output;
}