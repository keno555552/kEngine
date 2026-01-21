#pragma once

enum class PSOID {
	NONE = -1,

	/// 2D系
	Sprite2D = 0,

	/// 3D系
	Lambert,
	HalfLambert,
	PhongReflection,
	BlinnPhongReflection,
	NUMBER_OF_PSO,

	/// デフォルトPSO
	defaultPSO = Lambert,
};

