#include "Model.h"
#include <d3dcompiler.h>
#include <DirectXTex.h>

#include "fstream"
#include "string"
#include "sstream"
#include "vector"

#pragma comment(lib, "d3dcompiler.lib")

Model* Model::GetInstance()
{
	static Model instance;
	return &instance;
}

void Model::Initialize(DirectXCommon* dx_, const std::string& filename, const std::string& resourcename)
{
	dx = dx_;
	HRESULT result;

	//頂点初期化
	InitializeVertex(filename);
	//デスクリプタ初期化
	InitializeDesc();
	//シェーダ読み込み
	CompileShader(L"ObjVertex.hlsl", L"ObjPixel.hlsl");
	//テクスチャ読み込み
	LoadTexture(resourcename);
}

void Model::InitializeVertex(const std::string& filename)
{
	HRESULT result;
	//ファイルストリーム
	std::ifstream file;
	const std::string modelname = filename;
	const std::string filename2 = modelname + ".obj";
	const std::string directoryPath = "Resources/" + modelname + "/";
	//objファイルを開く
	file.open(directoryPath + filename2);
	assert(!file.fail());
	
	//ファイル読み込み
	std::vector<XMFLOAT3>positions;	//頂点座標
	std::vector<XMFLOAT3>normals;	//法線ベクトル
	std::vector<XMFLOAT2>texcoords;	//テクスチャIV

	//1行ずつ読み込む
	std::string  line;
	while (getline(file, line))
	{
		//1行分の文字列をストリームに変換して解析しやすくする
		std::istringstream line_stream(line);
		//半角スペース区切りで行の先頭文字列を取得
		std::string key;
		getline(line_stream, key, ' ');

		//マテリアル
		if (key == "material")
		{
			//マテリアルのファイル名読み込み
			std::string filename;
			line_stream >> filename;
			//マテリアル読み込み
			LoadMaterial(directoryPath, filename);
		}
		//頂点座標
		if (key == "v")
		{
			//XYZ座標読み込み
			XMFLOAT3 position{};
			line_stream >> position.x;
			line_stream >> position.y;
			line_stream >> position.z;
			//座標データに追加
			positions.emplace_back(position);
		}
		//テクスチャ
		if (key == "vt")
		{
			//U,V成分読み込み
			XMFLOAT2 texcoord{};
			line_stream >> texcoord.x;
			line_stream >> texcoord.y;
			//V方向反転
			texcoord.y = 1 - texcoord.y;
			//V方向反転
			texcoords.emplace_back(texcoord);
		}
		//法線ベクトル
		if (key == "vn")
		{
			XMFLOAT3 normal{};
			line_stream >> normal.x;
			line_stream >> normal.y;
			line_stream >> normal.z;
			//法線ベクトルデータに追加
			normals.emplace_back(normal);
		}
		//ポリゴン
		if (key == "f")
		{
			//半角スペース区切りで行の続きを読み込む
			std::string index_string;
			while (getline(line_stream, index_string, ' '))
			{
				///頂点インデックス1個分の文字列をストリームに変換して解析しやすくする
				std::istringstream index_stream(index_string);
				unsigned short indexPosition, indexNormal, indexTexcoord;
				index_stream >> indexPosition;
				index_stream.seekg(1, std::ios_base::cur);	//スラッシュを飛ばす
				index_stream >> indexTexcoord;
				index_stream.seekg(1, std::ios_base::cur);	//スラッシュを飛ばす
				index_stream >> indexNormal;
				//頂点データの追加
				VertexPosNormalUv vertex{};
				vertex.pos = positions[indexPosition - 1];
				vertex.normal = normals[indexNormal - 1];
				vertex.uv = texcoords[indexTexcoord - 1];
				vertices.emplace_back(vertex);
				//頂点インデックスに追加
				indices.emplace_back((unsigned short)indices.size());
			}
		}
	}
	//ファイルを閉じる
	file.close();

	UINT sizeVB = static_cast<UINT>(sizeof(VertexPosNormalUv) * vertices.size());

	// ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// リソース設定
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeVB);

	// 頂点バッファ生成
	result = dx->GetDevice()->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vertBuff));
	assert(SUCCEEDED(result));

	// 頂点バッファへのデータ転送
	VertexPosNormalUv* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (SUCCEEDED(result)) {
		std::copy(vertices.begin(), vertices.end(), vertMap);
		vertBuff->Unmap(0, nullptr);
	}

	// 頂点バッファビューの作成
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]);

	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * indices.size());
	// リソース設定
	resourceDesc.Width = sizeIB;

	// インデックスバッファ生成
	result = dx->GetDevice()->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&indexBuff));

	// インデックスバッファへのデータ転送
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	if (SUCCEEDED(result)) {

		std::copy(indices.begin(), indices.end(), indexMap);

		indexBuff->Unmap(0, nullptr);
	}

	// インデックスバッファビューの作成
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;
}

void Model::InitializeDesc()
{
	HRESULT result;

	// デスクリプタヒープを生成	
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
	descHeapDesc.NumDescriptors = 1; // シェーダーリソースビュー1つ
	result = dx->GetDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap));//生成
	if (FAILED(result)) {
		assert(0);
	}

	// デスクリプタサイズを取得
	descriptorHandleIncrementSize = dx->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void Model::CompileShader(const wchar_t* file, const wchar_t* file2)
{
	ComPtr<ID3DBlob> vsBlob;	//頂点シェーダーオブジェクト
	ComPtr<ID3DBlob> psBlob;	//頂点シェーダーオブジェクト
	ComPtr<ID3DBlob> errorBlob;	//エラーオブジェクト

	HRESULT result;

	//頂点シェーダーの読み込みとコンパイル
	result = D3DCompileFromFile(
		file,	//シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//インクルード可能にする
		"main",	//エントリー名
		"vs_5_0",	//シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//デバッグ用設定
		0,
		&vsBlob,
		&errorBlob
	);

	//エラーなら
	if (FAILED(result))
	{
		//errorBlobからエラーの内容をstring型にコピー
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n(
			(char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin()
		);
		error += "\n";
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	vsBlob = vsBlob;
	errorBlob = errorBlob;

	ID3DBlob* psBlob_ = nullptr;	//頂点シェーダーオブジェクト

	//ピクセルシェーダーの読み込みとコンパイル
	result = D3DCompileFromFile(
		file2,	//シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//インクルード可能にする
		"main",		//エントリーポイント名
		"ps_5_0",	//シェーダモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//デバッグ用設定
		0,
		&psBlob,
		&errorBlob
	);

	//エラーなら
	if (FAILED(result))
	{
		//errorBlobからエラーの内容をstring型にコピー
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n(
			(char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin()
		);
		error += "\n";
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	//頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{	//xyz座標
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		//normalize
		{
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		//uv
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	// グラフィックスパイプラインの流れを設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

	// サンプルマスク
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定
	// ラスタライザステート
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	// デプスステンシルステート
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	// レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// RBGA全てのチャンネルを描画
	blenddesc.BlendEnable = true;
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;

	// ブレンドステートの設定
	gpipeline.BlendState.RenderTarget[0] = blenddesc;

	// 深度バッファのフォーマット
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// 頂点レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	// 図形の形状設定（三角形）
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;	// 描画対象は1つ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0～255指定のRGBA
	gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

	// デスクリプタレンジ
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ

	// ルートパラメータ
	CD3DX12_ROOT_PARAMETER rootparams[3];
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[2].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	// スタティックサンプラー
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	// ルートシグネチャの設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob;
	// バージョン自動判定のシリアライズ
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	// ルートシグネチャの生成
	result = dx->GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
	assert(SUCCEEDED(result));

	gpipeline.pRootSignature = rootsignature.Get();

	// グラフィックスパイプラインの生成
	result = dx->GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelinestate));
	assert(SUCCEEDED(result));
}

void Model::LoadMaterial(const std::string& directoryPath, const std::string& filename)
{
	//ファイルストリーム
	std::ifstream file;
	//マテリアルファイルを開く
	file.open(directoryPath + filename);
	//ファイルオープン失敗をチェック
	if (file.fail())
	{
		assert(0);
	}
	//1行ずつ読み込む
	std::string line;
	while (getline(file, line))
	{
		//1行分の文字列をストリームに変換
		std::istringstream line_stream(line);
		//半角スペース区切りで行の先頭文字列を取得
		std::string key;
		getline(line_stream, key, ' ');
		//先頭のタブ文字は無視する
		if (key[0] == '\t')
		{
			key.erase(key.begin());
		}
		//マテリアル名
		if (key == "newmtl")
		{
			line_stream >> material.name;
		}
		//アンビエント色
		if (key == "Ka")
		{
			line_stream >> material.ambient.x;
			line_stream >> material.ambient.y;
			line_stream >> material.ambient.z;
		}
		//ディフューズ色
		if (key == "Kd")
		{
			line_stream >> material.diffuse.x;
			line_stream >> material.diffuse.y;
			line_stream >> material.diffuse.z;
		}
		//スペキュラー色
		if (key == "Ks")
		{
			line_stream >> material.specular.x;
			line_stream >> material.specular.y;
			line_stream >> material.specular.z;
		}
		//テクスチャファイル名
		if (key == "map_Kd")
		{
			line_stream >> material.textureFilename;
			/*LoadTexture(directoryPath, material.textureFilename);*/
		}
	}
	//ファイルを閉じる
	file.close();
}

void Model::LoadTexture(const std::string& resourcename)
{
	HRESULT result = S_FALSE;

	TexMetadata metadata{};
	ScratchImage scratchImg{};

	//ユニコード文字列に変換する
	wchar_t wfilepath[128];
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0,
		resourcename.c_str(), -1, wfilepath, _countof(wfilepath));

	// WICテクスチャのロード
	result = LoadFromWICFile(wfilepath, WIC_FLAGS_NONE, &metadata, scratchImg);

	ScratchImage mipChain{};
	// ミップマップ生成
	result = GenerateMipMaps(
		scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain);
	if (SUCCEEDED(result)) {
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}

	// 読み込んだディフューズテクスチャをSRGBとして扱う
	metadata.format = MakeSRGB(metadata.format);

	// リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format, metadata.width, (UINT)metadata.height, (UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels);

	// ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProps =
		CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);

	// テクスチャ用バッファの生成
	result = dx->GetDevice()->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // テクスチャ用指定
		nullptr, IID_PPV_ARGS(&texbuff));
	assert(SUCCEEDED(result));

	// テクスチャバッファにデータ転送
	for (size_t i = 0; i < metadata.mipLevels; i++) {
		const Image* img = scratchImg.GetImage(i, 0, 0); // 生データ抽出
		result = texbuff->WriteToSubresource(
			(UINT)i,
			nullptr,              // 全領域へコピー
			img->pixels,          // 元データアドレス
			(UINT)img->rowPitch,  // 1ラインサイズ
			(UINT)img->slicePitch // 1枚サイズ
		);
		assert(SUCCEEDED(result));
	}

	// シェーダリソースビュー作成
	cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap->GetCPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);
	gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(descHeap->GetGPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体
	D3D12_RESOURCE_DESC resDesc = texbuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	dx->GetDevice()->CreateShaderResourceView(texbuff.Get(), //ビューと関連付けるバッファ
		&srvDesc, //テクスチャ設定情報
		cpuDescHandleSRV
	);
}
