#include "CommonTypes.hlsli"

// ==============================
// Constant Buffers
// ==============================

StructuredBuffer<TransformationMatrix> gTransformationMatrices : register(t0);
StructuredBuffer<uint> gMaterialIndexList : register(t1);

cbuffer OffSetGroup : register(b0)
{
    uint gWVPOffset;
    uint gMaterialIndexListOffset;
};

// ==============================
// Vertex Input / Output
// ==============================

// ネオン用：PSに「ローカルY」を渡す
struct FlameVertexShaderOutput
{
    float4 position : SV_Position;
    float3 normal : NORMAL0;
    float3 worldPosition : POSITION0;
    float  localY   : TEXCOORD0;
    nointerpolation int materialId : TEXCOORD1;
};

FlameVertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    TransformationMatrix transform = gTransformationMatrices[gWVPOffset + instanceId];
    
    // Unity版は「ローカル座標のY」を使ってグラデーションしているので同じにする
    FlameVertexShaderOutput output;
    output.position = mul(input.position, transform.WVP);
    //output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) transform.worldInverseTranspose));
    output.worldPosition = mul(input.position, transform.world).xyz;
    output.localY = input.position.y;
    output.materialId = gMaterialIndexList[gMaterialIndexListOffset + instanceId];

    return output;
}
