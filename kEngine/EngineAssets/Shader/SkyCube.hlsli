
struct SkyCubeVSOutput
{
    float4 position : SV_POSITION;
    float3 texcoord : TEXCOORD0;
    nointerpolation int materialId : TEXCOORD1;
};

/// 速度のために、MaterialはgMaterialList[input.materialId].layers[0]しか使えない
/// そもそも背景は1層しか使わないので、これで十分