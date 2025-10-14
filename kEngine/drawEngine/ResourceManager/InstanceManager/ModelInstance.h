#pragma once
#include "Matrix4x4.h"

struct ModelInstance
{
	Matrix4x4 WVP;
	Matrix4x4 world;
	int materialConfigIndex;
	int drawState;

	bool CheckSame(const ModelInstance target);
};

