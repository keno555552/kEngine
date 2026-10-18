//-------------------------------------------------------------------------------------
// BCDirectCompute.cpp (stubbed)
//
// Direct3D 11 Compute Shader BC Compressor
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//-------------------------------------------------------------------------------------

#include "DirectXTexP.h"
#include "BCDirectCompute.h"

using namespace DirectX;

GPUCompressBC::GPUCompressBC() noexcept = default;

HRESULT GPUCompressBC::Initialize(_In_ ID3D11Device*) {
	return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
}

HRESULT GPUCompressBC::Prepare(size_t, size_t, uint32_t, DXGI_FORMAT, float) {
	return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
}

HRESULT GPUCompressBC::Compress(const Image&, const Image&) {
	return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
}
