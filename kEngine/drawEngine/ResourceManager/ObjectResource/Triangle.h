#pragma once
#include "VertexData.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include "VertexResource.h"
#include "Config.h"

class Triangle:public MeshBuffer
{
public:
	ID3D12Resource* CreateVertexResource_(ID3D12Device* device)override;

private:
	void SetWVP(TransformationMatrix* wvpData);
};

