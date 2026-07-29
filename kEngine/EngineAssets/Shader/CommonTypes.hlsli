

/// =========================== Vertex Shader Input/Output =========================== ///

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VertexShaderOutput
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPosition : POSITION0;
    nointerpolation int materialId : TEXCOORD1;
};

/// ============================== PixelShaderOutput ================================= ///

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

/// ===================================== 共通 ======================================== ///

/// ==================================== VS関連 ======================================= ///

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 world;
    float4x4 worldInverseTranspose;
};

/// ==================================== PS関連 ======================================= ///
/// ========================= Material
struct MaterialLayer
{
    float4 color;
    float4 color2;
    float4 color3;
    
    float4x4 uvTransform;
    
    float shininess;
    float intensity;
    float heightScale;
    float reflectiveStrength;
    
    int enableLighting;
    float padding[3];
};

struct MaterialGroup
{
    MaterialLayer layers[3];
    int layerCount;
    int enableLighting;
    float padding[2];
};

/// ====================================

struct LightGPU
{
    float3 direction;
    float angle;

    float3 position;
    float range;

    float3 color;
    float intensity;

    int type;
    int extra0;
    int extra1;
    int extra2;
};

struct Camera
{
    float3 position;
};

/// ===================================
/// Object3Dルート定義
/// ===================================
/// ======== VS:
/// #b
/// cbuffer InstanceOffset : register                                           (b0);
/// #t                                                                              
/// StructuredBuffer<TransformationMatrix> gTransformationMatrices : register   (t0);
/// StructuredBuffer<uint> gMaterialIndexList : register                        (t1);
/// ======== PS:                                                                    
/// #b                                                                              
/// ConstantBuffer<Camera> gCamera : register                                   (b0);
/// cbuffer LightCountCB : register                                             (b1);
/// #t                                                                              
/// Texture2D<float4> gTexture : register                                       (t0);
/// StructuredBuffer<MaterialGroup> gMaterialList : register                    (t1);
/// StructuredBuffer<LightGPU> gLights : register                               (t2);
/// TextureCube<float4> gEnvironmentTexture : register                          (t3);
/// #s                                                                              
/// SamplerState gSampler : register                                            (s0);

/// ===================================
/// Skinning
/// ===================================
/// ======== VS:
/// #t
/// StructuredBuffer<Well> gMatrixPalette : register                            (t2);
