#include "Particle.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 world;
};
StructuredBuffer<TransformationMatrix> gTransformationMatrix : register(t0);

cbuffer InstanceOffset : register(b1)
{
    uint instanceOffset;
}

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};


VertexShaderOutput main(VertexShaderInput input, uint instanceId:SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix[instanceId + instanceOffset].WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix[instanceId + instanceOffset].world));
    return output;
}


