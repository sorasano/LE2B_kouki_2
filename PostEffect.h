#pragma once
#include <Windows.h>
#include <D3dx12.h>

#include <DirectXMath.h>
using namespace DirectX;

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <wrl.h>

using namespace Microsoft::WRL;

#include <cassert>

#include <DirectXTex.h>

#include "DirectXCommon.h"

#include "Sprite.h"

class PostEffect
{
	//-----------スプライト----------

public:


	void SetPiplineSet(PipelineSet piplineSet);

	PipelineSet SpriteCreateGraphicsPipeline(ID3D12Device* dev);

	//スプライト1枚分のデータ

private:
	//頂点バッファ
	ComPtr<ID3D12Resource> vertBuff;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//定数バッファ
	ComPtr<ID3D12Resource> constBuff;
	//Z軸回りの回転角
	float rotation = 0.0f;
	//座標
	XMFLOAT3 position = { 0,0,0 };
	//ワールド行列
	XMMATRIX matWorld;

	UINT texNumber = 0;

	XMFLOAT2 size = { 100,100 };

	float time;

	struct ConstBufferData {
		XMFLOAT4 color; // 色 (RGBA)
		XMMATRIX mat; //座標
		float time;
	};

	//コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> cmdList;
	DirectXCommon* dx = nullptr;
	ComPtr<ID3D12DescriptorHeap> descHeap;
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};

public:

	//スプライト生成
	PostEffect SpriteCreate(ID3D12Device* dev, int window_width, int window_height);

	//スプライト共通データ生成
	PostEffect SpriteCommonCreate(ID3D12Device* dev, int window_width, int window_height);

	//スプライト共通グラフィックスコマンドのセット
	void SpriteCommonBeginDraw(ID3D12GraphicsCommandList* cmdList, const SpriteCommon& spriteCommon);

	//スプライト単体描画

	void SpriteDraw(ID3D12GraphicsCommandList* cmdList, const PostEffect& sprite, const SpriteCommon& spriteCommon, ID3D12Device* dev);

	//スプライト単体更新
	void SpriteUpdate(PostEffect& sprite, const SpriteCommon& spriteCommon);

	//スプライト共通テクスチャ読み込み
	void SpriteCommonLoadTexture(SpriteCommon& spriteCommon, UINT texnumber, const wchar_t* filename, ID3D12Device* dev);

	//スプライト単体頂点バッファの転送
	void SpriteTransferVertexBuffer(const PostEffect& sprite);

	//セッター
	void SetTexNumber(UINT texnumber) { this->texNumber = texnumber; }

	void SetPosition(XMFLOAT3 position) { this->position = position; }
	void SetScale(XMFLOAT2 scale) { this->size = scale; }


	void Release();
};

