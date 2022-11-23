#pragma once
#include <Windows.h>
#include <cstdlib>
#pragma once
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dx12.h>
#include <string>
#include <DirectXCommon.h>

using namespace Microsoft::WRL;
using namespace DirectX;

struct PeraVertex {
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};


class Pera
{
public:

	void Initialize(ComPtr<ID3D12Device> device, DirectXCommon* dx);

	void Update();

	void Draw();

private:

	struct ConstBufferData {
		int mode;
		float time;
	};

	int mode = 1;
	float time = 0;

	PeraVertex pv[4] = {
	{{-1,-1,0.1},{0,1}},
	{{-1,+1,0.1},{0,0}},
	{{+1,-1,0.1},{1,1}},
	{{+1,+1,0.1},{1,0}}
	};

	DirectXCommon* dx = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList;

	//ComPtr<ID3D12Resource> _peraResource;
	//ComPtr<ID3D12DescriptorHeap> _peraRTVHeap; // レンダーターゲット用
	ComPtr<ID3D12DescriptorHeap> _peraSRVHeap; //テクスチャ用
	ComPtr<ID3D12RootSignature> _peraRS;
	ComPtr<ID3D12PipelineState> _peraPipeline;
	D3D12_VERTEX_BUFFER_VIEW  _peraVBV;
	ComPtr<ID3D12Resource> _peraVB;
	//定数バッファ
	ComPtr<ID3D12Resource> constBuff;

};

