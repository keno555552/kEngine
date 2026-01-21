#include "Particle.hlsli"
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

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
};

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

struct Camera
{
    float3 position;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    if (gMaterial.enableLighting)
    {
        float3 result = { 0, 0, 0 };
        float totalWeight = 0.0f;
        
#if defined(LIGHT_MODEL_LAMBERT)
        result += ApplyLighting_Lambert(normalize(input.normal), (-gDirectionalLight.direction), gDirectionalLight.color.rgb);
        totalWeight += 1.0f;
#endif
        
#if defined(LIGHT_MODEL_HALF)
        result += ApplyLighting_HalfLambert(normalize(input.normal), (-gDirectionalLight.direction), gDirectionalLight.color.rgb);
        totalWeight += 1.0f;
#endif
       
#if defined (LIGHT_MODEL_PHONG)

        result += ApplyLighting_PhongReflection(normalize(input.normal),
                                                (-gDirectionalLight.direction),
                                                gDirectionalLight.color.rgb,
                                                input.worldPosition,
                                                gCamera.position,
                                                gMaterial.shininess);
        totalWeight += 1.0f;
#endif 
       
#if defined (LIGHT_MODEL_BLINN_PHONG)

        result += ApplyLighting_BlinnPhongReflection(normalize(input.normal),
                                                    (-gDirectionalLight.direction),
                                                    gDirectionalLight.color.rgb,
                                                    input.worldPosition,
                                                    gCamera.position,
                                                    gMaterial.shininess);
        totalWeight += 1.0f;
#endif 
       
        //result += gMaterial.color.rgb * textureColor.rgb;
        //totalWeight += 1.0f;
        
        float3 lightFactor = { result / max(totalWeight, 1.0f) };
        //lightFactor = (gMaterial.color.rgb + lightFactor) * 0.5f;
        //float3 finalColor = textureColor.rgb * lightFactor * gDirectionalLight.intensity;
        float3 finalColor = gMaterial.color.rgb * textureColor.rgb * lightFactor * gDirectionalLight.intensity;
       
        output.color.rgb = finalColor;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    {
        output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.intensity;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    //output.color.rgb = normalize(input.normal) * 0.5f + 0.5f;
    //output.color.a = 1.0f;
    return output;

}

