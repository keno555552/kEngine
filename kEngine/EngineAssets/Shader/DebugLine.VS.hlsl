#include "CommonTypes.hlsli"
#include "DebugLine.hlsli"

StructuredBuffer<TransformationMatrix> gTransformationMatrices : register(t0);

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

DebugLineVSOutput main(VSInput input, uint instanceID : SV_InstanceID)
{
    DebugLineVSOutput o;
    
    TransformationMatrix m = gTransformationMatrices[instanceID];
    
    o.position = mul(float4(input.position, 1.0f), m.WVP);

    o.color = input.color;
    return o;
}