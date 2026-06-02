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
    int kernelSize;
    int kernelIndex;
};

/// BlurType:(手動更新:260602)
/// Box,
/// Custom,
/// NumOfBlur


struct BlurData
{
    float kKernel[49];
};