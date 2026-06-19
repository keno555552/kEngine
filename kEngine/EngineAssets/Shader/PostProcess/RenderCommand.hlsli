struct RenderCommand
{
    // guardColor
    float3 guardColor;
    float guardAmount;
    
    // Vignette
    float2 vignetteCenter;
    float vignetteRadius;
    float vignetteSoftness;
    float vignetteIntensity;
    float4 vignetteColor;
    
    // blur
    int blurType;
    int blurKernelSize;
    int blurKernelIndex;
    
    // outline
    int outlineType;
    int outlineKernelSize;
    int outlineKernelIndex;
    float depthThreshold;
    
    float4 outlineColor;
    
};

/// BlurType:(手動更新:260602)
/// Box,
/// Custom,
/// NumOfBlur


struct BlurData
{
    float kKernel[49];
};