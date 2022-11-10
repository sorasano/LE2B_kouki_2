#pragma once
#include "Vertex.h"
#include "DirectXCommon.h"

class VertBuff
{
public:
	static VertBuff* GetInstance();
	void Initialize(Ver* vertex, DirectXCommon* dx_);
	void Initialize(Ver2* vertex, DirectXCommon* dx_);
	void Initialize(Ver3* vertex, DirectXCommon* dx_);
	void Update(Ver3 *vertex,DirectXCommon* dx_);
	D3D12_VERTEX_BUFFER_VIEW *GetVbView() { return &vbView; }
public:
	ComPtr<ID3D12Resource> vertBuff;
	D3D12_VERTEX_BUFFER_VIEW vbView{};		//頂点バッファビュー
};

