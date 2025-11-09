#pragma once
#include "TransformationMatrix.h"
#include "Matrix4x4.h"

struct ModelInstance
{
	TransformationMatrix* transformData;
	int materialConfigIndex;
	int resourceHandle;
	int modelHandle; /// modelHandleのハンドル
	bool useDefaultModel;
	int vertexNum;
	int drawState;

	bool CheckSame(const ModelInstance target);
};

