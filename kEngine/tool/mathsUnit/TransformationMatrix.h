#pragma once
#include "Matrix4x4.h"

struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 world;

	bool operator== (const TransformationMatrix target);
	bool operator!= (const TransformationMatrix target);
};

