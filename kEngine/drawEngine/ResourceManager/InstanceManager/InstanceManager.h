#pragma once
#include <vector>
#include <memory>
#include "MathsIncluder.h"
#include "material.h"
#include "materialConfig.h"
#include "TransformationMatrix.h"
#include "SpriteInstance.h"

class InstanceManager
{
public:
	~InstanceManager();

	void Update();

	void AddSpriteInstance(Vector2 pos, MaterialConfig material);
	void AddTileInstance(Vector2 pos, MaterialConfig material);
	//void ResmoveSpriteInstance();
	//void UpdateTileInstance();
	void AddModelInstance(TransformationMatrix* wvpData, MaterialConfig material);
	void AddParticleInstance(TransformationMatrix* wvpData, MaterialConfig material);
	//void ResmoveModelInstance();
	//void UpdateModelInstance();

public:

	enum DrawState {
		STANDBY,
		ISDRAW,
		DRAWLAST,
	};

	struct ModelInstance {
		TransformationMatrix transformMatrix;
		int modelID;
		int layer;
	};

public:
	std::vector< MaterialConfig* > materialConfigList_;
	std::vector< SpriteInstance* > spriteList_;
	std::vector< SpriteInstance* > tileList_;
	std::vector< ModelInstance *>  modelList_;

public:
	int layerCount = 0;
};
