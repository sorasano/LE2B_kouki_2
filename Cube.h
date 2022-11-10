#pragma once
#include "DirectXCommon.h"
#include "VertBuff.h"
#include "Object3D.h"
#include "d3d12.h"
#include "list"
#include "vector"
#include "d3dcompiler.h"

using namespace DirectX;
using namespace Microsoft::WRL;

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dinput8.lib")

#define PI 3.14159265359

class Cube
{
public:
	//シングルトンインスタンス
	static Cube* GetInstance();
	//初期化
	void Initialize(XMFLOAT3 size, DirectXCommon* dx_);
	void InitializeVertex(XMFLOAT3 size);	//頂点初期化
	void InitializeIndexBuff();				//インデックスバッファ初期化	
	void InitializeVertBuff();				//頂点バッファ初期化
	void CompileShader(const wchar_t* file, const wchar_t* file2);	//シェーダ読み込み
	void InitializeRootSignature();			//ルートシグネチャ
	void InitializePipeline();				//パイプライン
	void InitializePipelineState();			//パイプラインステート
	//更新
	void Update();
public:
	DirectXCommon* dx;
	ComPtr<ID3D12PipelineState> pipelineState;
	ComPtr<ID3D12Resource> constBuffMaterial;
public:
	D3D12_RECT scissorRect{};
	D3D12_VIEWPORT viewport{};

public:
	//頂点データ構造体
	struct Vertex
	{
		XMFLOAT3 pos;	//座標
		XMFLOAT3 normalize;	//法線ベクトル
		XMFLOAT2 uv;	//uv座標
		Vertex* parent = nullptr;
	};

	struct Indices
	{
		int num;
	};
	//Vertex関連
	size_t verticesCount = 24;
	std::vector<Vertex> vertices;	//外部に渡す用の頂点データ
	size_t indicesCount = 36;
	std::vector<unsigned short> indices;
	UINT sizeVB;
	UINT sizeIB;
	D3D12_INPUT_ELEMENT_DESC inputLayout[3];//頂点レイアウト	xyz座標、法線ベクトル、uv座標の順番
	//インデックスバッファ関連
	ComPtr<ID3D12Resource> indexBuff;
	D3D12_INDEX_BUFFER_VIEW ibView{};
	//頂点バッファ初期化
	ComPtr<ID3D12Resource> vertBuff;
	D3D12_VERTEX_BUFFER_VIEW vbView{};		//頂点バッファビュー
	//シェーダ関連
	ID3DBlob* vsBlob;	//頂点シェーダーオブジェクト
	ID3DBlob* psBlob;	//ピクセルシェーダーオブジェクト
	ID3DBlob* errorBlob;	//エラーオブジェクト
	//ルートシグネチャ
	ComPtr<ID3D12RootSignature> rootSignature;
	//パイプライン
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};	//グラフィックスパイプライン
};

