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
	ID3D12Resource* SetWVPResource_(ID3D12Device* device, TransformationMatrix* wvpData);

private:
	void SetWVP(TransformationMatrix* wvpData);
};

