#ifndef _LM_BlinnPHONG_LIGHTING_
#define _LM_BlinnPHONG_LIGHTING_

float3 ApplyLighting_PhongReflection(float3 N, float3 L, float3 lightColor, float3 worldPos, float3 cameraPos, float shininess)
{
    N = normalize(N);
    L = normalize(L);
    
    // View direction
    float3 toEye = normalize(cameraPos - worldPos);
    
    // Diffuse (Half-Lambert)
    float NdotL = dot(N, L);
    float diffuse = pow(NdotL * 0.5f + 0.5f, 2.0f);
    
    // Specular (Phong)
    float3 reflectedLight = reflect(-L, N); // 注意 reflect 的第一個參數要是 -L
    float RdotE = max(dot(reflectedLight, toEye), 0.0f);
    float specularPow = pow(saturate(RdotE), shininess);
    
    // Combine
    return (diffuse + specularPow) * lightColor;
}

#endif
