#pragma once
#include "Vertex.h"
#include "DirectXCommon.h"

class IndexBuff
{
public:
	static IndexBuff* GetInstance();
	void Initialize(Ver* vertex, DirectXCommon* dx_);
	void Initialize(Ver2* vertex, DirectXCommon* dx_);
	void Initialize(Ver3* vertex, DirectXCommon* dx_);
	D3D12_INDEX_BUFFER_VIEW* GetIbView() { return &ibView; }
public:
	ComPtr<ID3D12Resource> indexBuff;
	D3D12_INDEX_BUFFER_VIEW ibView{};
};

