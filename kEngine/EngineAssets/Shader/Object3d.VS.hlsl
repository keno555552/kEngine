#include "CommonTypes.hlsli"

StructuredBuffer<TransformationMatrix> gTransformationMatrices : register(t0);
StructuredBuffer<uint> gMaterialIndexList : register(t1);

cbuffer OffSetGroup : register(b0)
{
    uint gWVPOffset;
    uint gMaterialIndexListOffset;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    TransformationMatrix transform = gTransformationMatrices[gWVPOffset + instanceId];
    
    VertexShaderOutput output;
    output.position = mul(input.position, transform.WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) transform.worldInverseTranspose));
    output.worldPosition = mul(input.position, transform.world).xyz;
    output.materialId = gMaterialIndexList[gMaterialIndexListOffset + instanceId];
    return output;
}