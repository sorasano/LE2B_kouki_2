#pragma once
#include "DirectXCommon.h"
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

const int fine = 64;	//球体の細かさ	変数宣言用
const int fine2 = fine * fine * 2;	//描画に使う頂点の数
const int fine3 = fine * fine * 3;	//インデックスの数
const int fine4 = fine * fine + fine;	//頂点の数

class Sphere
{
public:
	//シングルトンインスタンス
	static Sphere* GetInstance();
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
	const float fineSize = fine;	//球体の細かさ
	//Vertex vertices[fine2];	//外部に渡す用の頂点データ
	std::vector<Vertex> vertices;	//外部に渡す用の頂点データ
	std::vector<Vertex> v;	
	std::vector<Vertex> v2;	
	std::vector<Vertex> v3;
	std::vector<unsigned short> indices;
	UINT sizeVB;
	UINT sizeIB;
	float angleX, angleY;
	float oneAngle = (2 * PI) / fineSize;
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

