#pragma once

/// ===================================
/// Object3Dルート定義
/// ===================================
/// b0:InstanceOffset			(VS)
/// t0:TransformMatrices		(VS)
/// t1:MaterialIndexList		(VS)
///=================================
/// b0:Camera					(PS)
/// b1:LightingCount			(PS)
///---------------------------------
/// t0:Texture					(PS)
/// t1:MaterialList				(PS)
/// t2:LightList				(PS)
/// t3:ENVIRONMENT REFLECTION	(PS)
///---------------------------------
/// s0:Sampler					(PS)
							

enum class RootSlot {
	MaterialList_SB				= 0,
	TransformMatricesList_SB	= 1,
	Texture_SRV					= 2,
	LightingCount_CB			= 3,
	InstanceOffset_CB			= 4,
	Camera_CB					= 5,
	LightList_SB				= 6,
	EnvironmentReflection_SRV	= 7,
	MaterialIndexList_SB		= 8,
};

/// ===================================
/// Skinning
/// ===================================
/// t2:BoneMatrices				(VS)

enum class RootSlotSkinning {
	MaterialList_SB				= 0,
	TransformMatricesList_SB	= 1,
	Texture_SRV					= 2,
	LightingCount_CB			= 3,
	InstanceOffset_CB			= 4,
	Camera_CB					= 5,
	LightList_SB				= 6,
	EnvironmentReflection_SRV	= 7,
	MaterialIndexList_SB		= 8,
	SkinningWell_SB				= 9,
};

/// ===================================
/// PostProcess
/// ===================================
/// s0:Sampler 				(PS)
/// s1:DepthSampler			(PS)
/// t0:SourceTexture		(PS)
/// t2:DepthTexture			(PS)
/// b0:RenderCommand		(PS)
/// t1:KernelData			(PS)
/// t3:Texture2用 			(PS)

enum class RootSlotPostProcess {
	SourceTexture_SB			= 0,
	RenderCommand_SB			= 1,
	KernelData_SB				= 2,
	DepthTexture_SRV			= 3,
	Texture2_SRV				= 4,
};

