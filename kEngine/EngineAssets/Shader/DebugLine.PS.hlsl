#include "CommonTypes.hlsli"
#include "DebugLine.hlsli"

PixelShaderOutput main(DebugLineVSOutput input)
{
    PixelShaderOutput output;
    output.color = input.color;
    return output;
}
