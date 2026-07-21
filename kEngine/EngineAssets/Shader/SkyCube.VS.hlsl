#include "CommonTypes.hlsli"
#include "SkyCube.hlsli"

StructuredBuffer<TransformationMatrix> gTransformationMatrices : register(t0);
StructuredBuffer<uint> gMaterialIndexList : register(t1);

cbuffer OffSetGroup : register(b1)
{
    uint gWVPOffset;
    uint gMaterialIndexListOffset;
};

SkyCubeVSOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    
    TransformationMatrix transform = gTransformationMatrices[gWVPOffset + instanceId];
    
    SkyCubeVSOutput o;
    
    o.position = mul(input.position, transform.WVP).xyww;
    o.texcoord = normalize(input.position.xyz);
    o.materialId = gMaterialIndexList[gMaterialIndexListOffset + instanceId];
    
    //float4 pos = mul(input.position, transform.WVP);
    //o.position = float4(pos.xy, pos.w, pos.w);
    //o.texcoord = normalize(input.position.xyz);
    
    return o;
}