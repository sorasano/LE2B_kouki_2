#include "Sphere.h"
#define PI 3.141592653589793238462643

using namespace DirectX;
using namespace Microsoft::WRL;


Sphere* Sphere::GetInstance()
{
	static Sphere instance;
	return &instance;
}

void Sphere::Initialize(XMFLOAT3 size, DirectXCommon* dx_)
{
	dx = dx_;
	HRESULT result;

	vertices.resize(fine2);
	v.resize(fine2);
	v2.resize(fine4);
	v3.resize(fine4); 
	indices.resize(fine3);

	//頂点初期化
	InitializeVertex(size);
	//インデックスバッファ初期化
	InitializeIndexBuff();
	//頂点バッファ初期化
	InitializeVertBuff();
	//シェーダ読み込み
	CompileShader(L"BasicVS.hlsl", L"BasicPS.hlsl");
	//ルートシグネチャ
	InitializeRootSignature();
	//パイプライン
	InitializePipeline();
	//パイプラインステート
	InitializePipelineState();


	//定数バッファ
	//ヒープ設定
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;	//GPUの転送用
	//リソース設定
	D3D12_RESOURCE_DESC cbResourceDesc{};
	cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbResourceDesc.Width = (sizeof(ConstBufferDataMaterial) + 0xff) & ~0xff;	//256バイトアラインメント
	cbResourceDesc.Height = 1;
	cbResourceDesc.DepthOrArraySize = 1;
	cbResourceDesc.MipLevels = 1;
	cbResourceDesc.SampleDesc.Count = 1;
	cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//定数バッファの生成
	result = dx->GetDevice()->CreateCommittedResource(
		&cbHeapProp,	//ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&cbResourceDesc,	//リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffMaterial)
	);
	assert(SUCCEEDED(result));

	//定数バッファのマッピング
	ConstBufferDataMaterial* constMapMaterial = nullptr;
	result = constBuffMaterial->Map(0, nullptr, (void**)&constMapMaterial);	//マッピング
	assert(SUCCEEDED(result));
	//値を書き込むと自動的に転送される
	constMapMaterial->color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}

#pragma region 頂点初期化
void Sphere::InitializeVertex(XMFLOAT3 size)
{
	angleY = 0;
	//頂点データ
	float x, y, z;
	for (int i = 0; i < fine2; i++)
	{
		if (i == 0 || i % 4 == 0)
		{
			if (i == 0)
			{
				angleX = 0;
			}
			if (i == 0 || i % (fine * 4) == 0)
			{
				angleY = (2 * PI) * ((float)(i + fine * 4) / (float)(fine * fine * 4));
			}
			else
			{
				angleY += oneAngle;
			}

			v[i].pos.x = size.x * cos(angleX) * sin(angleY);
			v[i].pos.y = size.y * cos(angleY);
			v[i].pos.z = size.z * sin(angleX) * sin(angleY);

		}

		if (i == 1 || i % 4 == 1)
		{
			angleY -= oneAngle;

			v[i].pos.x = size.x * cos(angleX) * sin(angleY);
			v[i].pos.y = size.y * cos(angleY);
			v[i].pos.z = size.z * sin(angleX) * sin(angleY);

		}
		if (i == 2 || i % 4 == 2)
		{
			angleX += oneAngle;
			angleY += oneAngle;

			v[i].pos.x = size.x * cos(angleX) * sin(angleY);
			v[i].pos.y = size.y * cos(angleY);
			v[i].pos.z = size.z * sin(angleX) * sin(angleY);

		}
		if (i == 3 || i % 4 == 3)
		{
			angleY -= oneAngle;

			v[i].pos.x = size.x * cos(angleX) * sin(angleY);
			v[i].pos.y = size.y * cos(angleY);
			v[i].pos.z = size.z * sin(angleX) * sin(angleY);

		}
	}

	unsigned short in[fine3];
	for (int i = 0; i < fine3; i++)
	{
		double num_ = ((i / 6) * 6) * 2 / 3;
		if (i == 0 || i % 6 == 0) { in[i] = num_; }
		if (i == 1 || i == 4 || i % 6 == 1 || i % 6 == 4) { in[i] = num_ + 1; }
		if (i == 2 || i == 3 || i % 6 == 2 || i % 6 == 3) { in[i] = num_ + 2; }
		if (i == 5 || i % 6 == 5) { in[i] = num_ + 3; }
	}

	angleY = 0;
	angleX = 0;
	//頂点データ	上から順番に割り当てる
	for (int i = 0; i < fine4; i++)
	{
		if (i == 0 || i % fine == 0)
		{
			angleX = 0;
		}
		else
		{
			angleX += oneAngle;
		}
		if (i == 0)
		{
			angleY = 0;
		}
		else if (i != 0 && i >= fine && i % fine == 0)
		{
			angleY = (2 * PI) * ((float)(i) / (float)(fine * fine));
		}
		v2[i].pos.x = size.x * cos(angleX) * sin(angleY);
		v2[i].pos.y = size.y * cos(angleY);
		v2[i].pos.z = size.z * sin(angleX) * sin(angleY);
		v3[i].pos.x = v2[i].pos.x;
		v3[i].pos.y = v2[i].pos.y;
		v3[i].pos.z = v2[i].pos.z;
	}


	for (int i = 0; i < fine2; i++)
	{
		for (int j = 0; j < fine4; j++)
		{
			//uv(0.0f,0.0f)
			if (i == 1 || i % 4 == 1)
			{
				if (i == 1)
				{
					v[i].parent = &v2[0];
				}
				else if (i % 4 == 1 && i != 1)
				{
					v[i].parent = &v2[i / 4];
				}
			}
			//uv(1.0f,0.0f)
			if (i == 3 || i % 4 == 3)
			{
				if (i == 3)
				{
					v[i].parent = &v2[1];
				}
				else if (i % 4 == 3)
				{
					if (i % (fine * 4) != (fine * 4) - 1 && i != (fine * 4) - 1)
					{
						v[i].parent = &v2[(i + 1) / 4];
					}
					if (i % (fine * 4) == (fine * 4) - 1 || i == (fine * 4) - 1)
					{
						v[i].parent = &v2[(i / 4) - (fine - 1)];
					}
				}
			}

			//uv(0.0f,1.0f)
			if (i == 0 || i % 4 == 0)
			{
				if (i == 0)
				{
					v[i].parent = &v2[fine];
				}
				else if (i % 4 == 0 && i != 0)
				{
					v[i].parent = &v2[(i / 4) + fine];
				}
			}

			if (i == 2 || i % 4 == 2)
			{
				if (i == 2)
				{
					v[i].parent = &v2[fine + 1];
				}
				else if (i % 4 == 2)
				{
					if (i % (fine * 4) != (fine * 4) - 2 && i != (fine * 4) - 2)
					{
						v[i].parent = &v2[(i + 2) / 4 + fine];
					}
					if (i % (fine * 4) == (fine * 4) - 2 || i == (fine * 4) - 2)
					{
						v[i].parent = &v2[(i / 4) - (fine - 1) + fine];
					}
				}
			}
		}
	}


	//頂点座標、uv座標、インデックスデータを代入
	for (int i = 0; i < fine2; i++)
	{
		vertices[i] = v[i];
	}

	for (int i = 0; i < fine3; i++)
	{
		indices[i] = in[i];
	}

	//法線の計算
	for (int i = 0; i < fine3 / 3; i++)
	{//三角形1つごとに計算していく
		//三角形のインデックスを取り出して、一時的な変数に入れる
		unsigned short indices0 = indices[i * 3 + 0];
		unsigned short indices1 = indices[i * 3 + 1];
		unsigned short indices2 = indices[i * 3 + 2];
		//三角形を構成する頂点座標をベクトルに代入
		XMVECTOR p0 = XMLoadFloat3(&vertices[indices0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[indices1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[indices2].pos);
		//p0→p1ベクトル、p0→p2ベクトルを計算　(ベクトルの減算)
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		//外積は両方から垂直なベクトル
		XMVECTOR normal = XMVector3Cross(v1, v2);
		//正規化
		normal = XMVector3Normalize(normal);
		//求めた法線を頂点データに代入
		XMStoreFloat3(&vertices[indices0].normalize, normal);
		XMStoreFloat3(&vertices[indices1].normalize, normal);
		XMStoreFloat3(&vertices[indices2].normalize, normal);
	}

	sizeVB = static_cast<UINT>(sizeof(vertices[0]) * vertices.size());
	sizeIB = static_cast<UINT>(sizeof(uint16_t) * indices.size());

	//頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout_[] =
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
	for (int i = 0; i < 3; i++)
	{
		inputLayout[i] = inputLayout_[i];
	}
}
#pragma endregion
#pragma region インデックスバッファ初期化
void Sphere::InitializeIndexBuff()
{
	HRESULT result;

	D3D12_HEAP_PROPERTIES heapProp{};	//ヒープ設定
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;	//CPUへの転送用
	D3D12_RESOURCE_DESC resDesc{};
	//リソース設定
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeIB;	//インデックス情報が入る分のサイズ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	result = dx->GetDevice()->CreateCommittedResource(
		&heapProp,	//ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resDesc,	//リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff)
	);

	//インデックスバッファをマッピング
	uint16_t* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	//全インデックスに対して
	for (int i = 0; i < indices.size(); i++)
	{
		indexMap[i] = indices[i];	//インデックスをコピー
	}
	//マッピング解除
	indexBuff->Unmap(0, nullptr);

	//インデックスバッファビューの作成
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;
}
#pragma endregion
#pragma region 頂点バッファ初期化
void Sphere::InitializeVertBuff()
{
	HRESULT result;

	//頂点バッファの設定
	D3D12_HEAP_PROPERTIES heapProp{};	//ヒープ設定
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;	//CPUへの転送用
	//リソース設定
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeVB;	//頂点データ全体のサイズ
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	result = dx->GetDevice()->CreateCommittedResource(
		&heapProp,	//ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resDesc,	//リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff)
	);
	assert(SUCCEEDED(result));

	//頂点バッファへのデータ転送
	//GPU上のバッファに対応した仮想メモリ（メインメモリ上）を取得
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));
	//全頂点に対して
	for (int i = 0; i < vertices.size(); i++)
	{
		vertMap[i] = vertices[i];	//座標をコピー
	}
	//つながりを解除
	vertBuff->Unmap(0, nullptr);

	//頂点バッファビューの作成
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]);
}
#pragma endregion
#pragma region シェーダ
void Sphere::CompileShader(const wchar_t* file, const wchar_t* file2)
{
	ID3DBlob* vsBlob_ = nullptr;	//頂点シェーダーオブジェクト
	ID3DBlob* errorBlob_ = nullptr;	//エラーオブジェクト

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
		&vsBlob_,
		&errorBlob_
	);

	//エラーなら
	if (FAILED(result))
	{
		//errorBlobからエラーの内容をstring型にコピー
		std::string error;
		error.resize(errorBlob_->GetBufferSize());

		std::copy_n(
			(char*)errorBlob_->GetBufferPointer(),
			errorBlob_->GetBufferSize(),
			error.begin()
		);
		error += "\n";
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	vsBlob = vsBlob_;
	errorBlob = errorBlob_;

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
		&psBlob_,
		&errorBlob_
	);

	//エラーなら
	if (FAILED(result))
	{
		//errorBlobからエラーの内容をstring型にコピー
		std::string error;
		error.resize(errorBlob_->GetBufferSize());

		std::copy_n(
			(char*)errorBlob_->GetBufferPointer(),
			errorBlob_->GetBufferSize(),
			error.begin()
		);
		error += "\n";
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	psBlob = psBlob_;
	errorBlob = errorBlob_;
}
#pragma endregion
#pragma region ルートシグネチャ
void Sphere::InitializeRootSignature()
{
	HRESULT result;

	//デスクリプタレンジの設定
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.NumDescriptors = 1;	//一度の描画に使うテクスチャが1枚なので1
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//ルートパラメータの設定
	D3D12_ROOT_PARAMETER rootParams[3] = {};
	//定数バッファ0番
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//種類
	rootParams[0].Descriptor.ShaderRegister = 0;					//定数バッファ番号
	rootParams[0].Descriptor.RegisterSpace = 0;						//デフォルト値
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	//全てのシェーダから見える
	//テクスチャレジスタ0番
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//種類
	rootParams[1].DescriptorTable.pDescriptorRanges = &descriptorRange;			//デスクリプタレンジ
	rootParams[1].DescriptorTable.NumDescriptorRanges = 1;						//デスクリプタレンジ数
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;				//全てのシェーダから見える
	//定数バッファ1番
	rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//種類
	rootParams[2].Descriptor.ShaderRegister = 1;					//定数バッファ番号
	rootParams[2].Descriptor.RegisterSpace = 0;						//デフォルト値
	rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	//全てのシェーダーから見える
	//テクスチャサンプラーの設定
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//横繰り返し
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//横繰り返し
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//奥行繰り返し
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;	//ボーダーの時は黒
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;					//全てリニア補間
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;						//ミップマップ最大値
	samplerDesc.MinLOD = 0.0f;									//ミップマップ最小値
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//ピクセルシェーダからのみ使用可能

	//ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParams;	//ルートパラメータの先頭アドレス
	rootSignatureDesc.NumParameters = _countof(rootParams);		//ルートパラメータ数
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	//ルートシグネチャのシリアライズ
	ComPtr<ID3DBlob> rootSigBlob;
	result = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		&errorBlob);
	assert(SUCCEEDED(result));
	result = dx->GetDevice()->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature)
	);
	assert(SUCCEEDED(result));
}
#pragma endregion
#pragma region パイプライン
void Sphere::InitializePipeline()
{
	HRESULT result;

	//シェーダをパイプラインに設定
	pipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	//サンプルマスクの設定
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;	//標準設定

	//ラスタライザの設定
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;	//カリングしない
	//pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;	//背面をカリング
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;	//ポリゴン塗りつぶし
	pipelineDesc.RasterizerState.DepthClipEnable = true;	//深度クリッピングを有効に

	//ブレンドステートを有効に
	pipelineDesc.BlendState.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;	//RGBA全てのチャンネルを描画

	//頂点レイアウトの設定
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);

	//図形の形状設定
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//その他の設定
	pipelineDesc.NumRenderTargets = 1;	//描画対象の数
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//0~255指定のRGBA
	pipelineDesc.SampleDesc.Count = 1;	//1ピクセルにつき1回サンプリング

	//デプスステンシルステートの設定
	pipelineDesc.DepthStencilState.DepthEnable = true;	//深度テストを行う
	pipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	//書き込み許可
	pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//パイプラインにルートシグネチャをセット
	pipelineDesc.pRootSignature = rootSignature.Get();

	/*result = dx->GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));*/
}
#pragma endregion
#pragma region パイプラインステート
void Sphere::InitializePipelineState()
{
	HRESULT result;
	result = dx->GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));
}
#pragma endregion

void Sphere::Update()
{
	HRESULT result;
	/*vertex->Update();
	vertBuff.Initialize(vertex, dx);*/
	pipelineDesc.pRootSignature = rootSignature.Get();
	//パイプラインステート
	/*result = dx->GetDevice()->CreateGraphicsPipelineState(&pipe.pipelineDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));*/
	//ビューポート設定コマンド
	viewport.Width = window_width;
	viewport.Height = window_height;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//ビューポート設定コマンドをコマンドリストに積む
	dx->GetCommandList()->RSSetViewports(1, &viewport);

	scissorRect.left = 0;
	scissorRect.right = scissorRect.left + window_width;
	scissorRect.top = 0;
	scissorRect.bottom = scissorRect.top + window_height;
	//シザー矩形設定コマンドをコマンドリストに積む
	dx->GetCommandList()->RSSetScissorRects(1, &scissorRect);
	//パイプラインステートをセット
	dx->GetCommandList()->SetPipelineState(pipelineState.Get());
	//ルートシグネチャをセット
	dx->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	//プリミティブ形状の設定コマンド
	dx->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	//三角形リスト
	//定数バッファビュー(CBV)の設定コマンド
	dx->GetCommandList()->SetGraphicsRootConstantBufferView(0, constBuffMaterial->GetGPUVirtualAddress());
}
