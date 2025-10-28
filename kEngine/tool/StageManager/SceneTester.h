#pragma once
#include "Scene.h"
#include "TransformationMatrix.h"
#include "Transform.h"			//Transform.h
#include "Vector4.h"
#include "Vector2.h"
#include <vector>
#include "MaterialConfig.h"
#include "DirectionalLight.h"
#include "config.h"
#include "DebugCamera.h"


class SceneTester :public Scene
{
public:
	SceneTester(kEngine* system);
	~SceneTester();
	void Update() override;
	void Draw() override;


private:

	enum objectType {
		TRIANGLE,
		SPRITE,
		TILE,
		CUBE,
		SPHERE,
		MODEL,
		BUNNY,
		TEAPOT,
		SUZANNE,
		MULTIMESH,
		MULTIMATERIAL,
		CHARATER,
		NEEDLE,
		SKYDOME,
		PRATICLE,
	};

	enum class ModelType {
		DEFAULT,
		BUNNY,
		TEAPOT,
		SUZANNE,
		MULTIMESH,
		MULTIMATERIAL,
		CHARATER,
		NEEDLE,
		SKYDOME,
		TILE3D,
		PRATICLE,
	};

	///Transform変数を作る
	struct Object {
		int objectType = TRIANGLE;

		Transform transform = CreateDefaultTransform();

		Matrix4x4 worldMatrix = {};

		Matrix4x4 worldViewProjectionMatrix{};

		TransformationMatrix transformationMatrix{};

		std::vector<MaterialConfig> materialConfig;
		//MaterialConfig materialConfig{

		ModelType modelHandle = ModelType::DEFAULT;

		bool isDelete = false;
	};


private:

	kEngine* system_ = nullptr;

	///Lighting
	DirectionalLight directionalLight{
		.color{1.0f,1.0f,1.0f,1.0f},
		.direction{-0.5f,-0.5f,0.4f},
		.intensity{1.0f}
	};

	///カメラ変数を作る
	DebugCamera* debugCamera_ = nullptr;

private:
	/// LoadModel
	int bunnyModelHandle_ = 0;
	int teapotModelHandle_ = 0;
	int suzanneModelHandle_ = 0;
	int multiMeshModelHandle_ = 0;
	int multiMaterialModelHandle_ = 0;
	int charaterModelHandle_ = 0;
	int needleModelHandle_ = 0;
	int skydomeModelHandle_ = 0;

private:
	/// TextureHandle
	int uvCheckerHandle1_ = 0;
	int monsterBallHandle_ = 0;
	int objHandle_ = 0;
	int whiteHandle_ = 0;

private:	
	/// sound
	int se1_ = 0;
	int se2_ = 0;


private:
	/// オブジェクト群
	std::vector<Object*> objectGroup;

	/// Test
	//Object* player = new Object;

private:
	/// Menu用変数
	bool isUVChecker = true;
	bool isMonsterBall = false;
	int CreateObjectHandle = 0;
	bool isTemplate = false;
	int effectType = 0;

	int textSubject = 6;

	float A = 0;
	float B = 0;
	int C = 0;
	int E = 0;
	int D = 0;


	float iVolume = 1.0f;
	float mVolume = 1.0f;
	float seVolume = 1.0f;
	float bgmVolume = 1.0f;

	bool iMute = false;
	bool masterMute = false;
	bool bgmMute = false;
	bool seMute = false;
};

