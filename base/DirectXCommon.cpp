#include "DirectXCommon.h"

//シングルトンインスタンスを取得
DirectXCommon* DirectXCommon::GetInstance()
{
	static DirectXCommon instance;
	return &instance;
}

//初期化処理
void DirectXCommon::Initialize(WinApp* winApp)
{
	winApp_ = winApp;
	HRESULT result;

	//デバイス初期化
	InitializeDevice();
	//コマンドリスト初期化
	InitializeCommand();
	//スワップチェーン初期化
	InitializeSwapchain();
	//レンダーターゲットビュー初期化
	InitializeRenderTargetView();
	//深度バッファ
	InitializeDepthBuffer();

	InitializeMultipassRendering();

	//フェンス生成
	InitializeFence();
}

#pragma region デバイス初期化
void DirectXCommon::InitializeDevice()
{
	HRESULT result;

#ifdef _DEBUG
	//デバッグレイヤーをオンに
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}

#endif

	//DXGIファクトリーの生成
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//アダプター
	//パフォーマンスが高いものから順に、すべてのアダプターを列挙する
	for (UINT i = 0;
		dxgiFactory->EnumAdapterByGpuPreference(
			i,
			DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND;
		i++)
	{
		//動的配列に追加する
		adapters.push_back(tmpAdapter);
	}

	//妥当なアダプターを選別する
	for (size_t i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC3 adapterDesc;
		//アダプターの情報を取得する
		adapters[i]->GetDesc3(&adapterDesc);

		//ソフトウェアデバイスを回避
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			//デバイスを採用してループを抜ける
			tmpAdapter = adapters[i];
			break;
		}
	}

	//対応レベルの生成
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (size_t i = 0; i < _countof(levels); i++)
	{
		//採用したアダプターをデバイスで生成
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i],
			IID_PPV_ARGS(&device));
		if (result == S_OK)
		{
			//デバイス生成できた時点でループを抜ける
			featureLevel = levels[i];
			break;
		}
	}
}
#pragma endregion
#pragma region コマンドリスト初期化
void DirectXCommon::InitializeCommand()
{
	HRESULT result;
	//コマンドアロケータを生成
	result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(result));

	//コマンドリストを生成
	result = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(result));

	//コマンドキューに設定
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//コマンドキューを生成
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));
}
#pragma endregion
#pragma region スワップチェーン
void DirectXCommon::InitializeSwapchain()
{
	HRESULT result;
	//スワップチェーンの設定
	swapChainDesc.Width = window_width;
	swapChainDesc.Height = window_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//色情報の書式
	swapChainDesc.SampleDesc.Count = 1;					//マルチサンプリングしない
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;	//バックバッファ用
	swapChainDesc.BufferCount = 2;						//バッファ数を2つに設定
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//フリップ後は破棄
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain1>swapchain1;

	//スワップチェーンの生成 
	result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(),
		winApp_->hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapchain1);

	swapchain1.As(&swapChain);
	assert(SUCCEEDED(result));
}
#pragma endregion
#pragma region レンダーターゲットビュー 
void DirectXCommon::InitializeRenderTargetView()
{
	// デスクリプタヒープの設定 
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー 
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount; //裏表の二つ

	// デスクリプタヒープの生成 
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	//バックバッファ
	backBuffers.resize(swapChainDesc.BufferCount);

	//スワップチェーンの全てのバッファについて処理する
	for (size_t i = 0; i < backBuffers.size(); i++)
	{
		//スワップチェーンからバッファを取得
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		//デスクリプタヒープのハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		//裏か表でアドレスがずれる
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//レンダーターゲットビューの設定
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		//シェーダーの計算結果をSRGBに変換して書き込む
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//レンダーターゲットビュの生成
		device->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, rtvHandle);
	}

	////マルチパスレンタリング

	////作成済みのヒープ情報を使ってもう一枚作る
	//auto heapDesc = rtvHeapDesc;

	////使っているバッファーの情報を利用する
	//auto& bbuff = backBuffers[0];
	//auto resDesc = bbuff->GetDesc();

	//D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	////レンタリング時のクリア時と同じ値
	//FLOAT clsClr[] = { 0.0f,0.0f,0.01f,0.0f };
	//D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);

	//auto result = device->CreateCommittedResource(&heapProp,
	//	D3D12_HEAP_FLAG_NONE,
	//	&resDesc,
	//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	//	&clearValue,
	//	IID_PPV_ARGS(_peraResource.ReleaseAndGetAddressOf()));

	////ビューを作る

	//ComPtr<ID3D12DescriptorHeap> _peraRTVHeap; // レンダーターゲット用
	//ComPtr<ID3D12DescriptorHeap> _peraSRVHeap; //テクスチャ用

	//// デスクリプタヒープの設定
	//srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // シェーダーリソースビュー 
	//srvHeapDesc.NumDescriptors = swapChainDesc.BufferCount; //裏表の二つ

	//// デスクリプタヒープの生成 
	//device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_peraRTVHeap));
	//device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&_peraSRVHeap));

	////RTV用ヒープを作る
	//heapDesc.NumDescriptors = 1;
	//result = device->CreateDescriptorHeap(
	//	&heapDesc,
	//	IID_PPV_ARGS(_peraResource.ReleaseAndGetAddressOf())
	//);

	//D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	//rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	//rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	////レンダーターゲットビューを作る

	//device->CreateRenderTargetView(
	//	_peraResource.Get(),
	//	&rtvDesc,
	//	_peraRTVHeap->GetCPUDescriptorHandleForHeapStart());

	////SRV用ヒープを作る
	//heapDesc.NumDescriptors = 1;
	//heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	//result = device->CreateDescriptorHeap(
	//	&heapDesc,
	//	IID_PPV_ARGS(_peraResource.ReleaseAndGetAddressOf())
	//);

	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//srvDesc.Format = rtvDesc.Format;
	//srvDesc.Texture2D.MipLevels = 1;
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	////シェーダーリソースビューを作る

	//device->CreateShaderResourceView(
	//	_peraResource.Get(),
	//	&srvDesc,
	//	_peraSRVHeap->GetCPUDescriptorHandleForHeapStart()
	//);

	////1パス目
	//auto rtvHeapPointer = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();

	//commandList->OMSetRenderTargets(
	//	1, &rtvHeapPointer, false,
	//	&dsvHeap->GetCPUDescriptorHandleForHeapStart());
}
#pragma endregion
#pragma region 深度バッファ
void DirectXCommon::InitializeDepthBuffer()
{
	HRESULT result;

	//リソース設定
	depthResorceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResorceDesc.Width = window_width;	//レンダーターゲットに合わせる
	depthResorceDesc.Height = window_height;	//レンダーターゲットに合わせる
	depthResorceDesc.DepthOrArraySize = 1;
	depthResorceDesc.Format = DXGI_FORMAT_D32_FLOAT;	//深度値フォーマット
	depthResorceDesc.SampleDesc.Count = 1;
	depthResorceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//デプスステンシル

	//震度値用ヒーププロパティ
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	//深度値のクリア設定
	depthClearValue.DepthStencil.Depth = 1.0f;	//深度値1.0f(最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;	//深度値フォーマット

	//リソース生成
	result = GetDevice()->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResorceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,	//深度値書き込みに使用
		&depthClearValue,
		IID_PPV_ARGS(&depthBuff)
	);

	//深度ビュー用デスクリプタヒープ作成
	dsvHeapDesc.NumDescriptors = 1;	//深度ビューは1つ
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//デプスステンシルビュー
	result = GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));

	//深度ステンシルビューの生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;	//深度値フォーマット
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	GetDevice()->CreateDepthStencilView(
		depthBuff.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);
}

void DirectXCommon::InitializeMultipassRendering()
{	
	HRESULT result;
	
	//マルチパスレンタリング

	//作成済みのヒープ情報を使ってもう一枚作る
	auto heapDesc = rtvHeapDesc;

	//使っているバッファーの情報を利用する
	auto& bbuff = backBuffers[0];
	auto resDesc = bbuff->GetDesc();

	D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	//レンタリング時のクリア時と同じ値
	FLOAT clsClr[] = { 0.0f,0.0f,0.01f,0.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);

	result = device->CreateCommittedResource(&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(_peraResource.ReleaseAndGetAddressOf()));

	//ビューを作る

	// デスクリプタヒープの設定
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // シェーダーリソースビュー 
	srvHeapDesc.NumDescriptors = swapChainDesc.BufferCount; //裏表の二つ

	// デスクリプタヒープの生成 
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_peraRTVHeap));
	device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&_peraSRVHeap));

	//RTV用ヒープを作る
	heapDesc.NumDescriptors = 1;
	result = device->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_peraResource.ReleaseAndGetAddressOf())
	);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//レンダーターゲットビューを作る

	device->CreateRenderTargetView(
		_peraResource.Get(),
		&rtvDesc,
		_peraRTVHeap->GetCPUDescriptorHandleForHeapStart());

	//SRV用ヒープを作る
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	result = device->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_peraResource.ReleaseAndGetAddressOf())
	);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = rtvDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//シェーダーリソースビューを作る

	device->CreateShaderResourceView(
		_peraResource.Get(),
		&srvDesc,
		_peraSRVHeap->GetCPUDescriptorHandleForHeapStart()
	);

	//ペラポリゴン生成

	struct PeraVertex {
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	PeraVertex pv[4] = {
		{{-1,-1,0.1},{0,1}},
		{{-1,+1,0.1},{0,0}},
		{{+1,-1,0.1},{1,1}},
		{{+1,+1,0.1},{1,0}}
	};

	ComPtr<ID3D12Resource> _peraVB;

	// リソース設定
	D3D12_HEAP_PROPERTIES peraHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	peraHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	peraHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	CD3DX12_RESOURCE_DESC peraResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(pv));

	peraResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	peraResourceDesc.Width = sizeof(pv);
	peraResourceDesc.Height = 1;
	peraResourceDesc.DepthOrArraySize = 1;
	peraResourceDesc.MipLevels = 1;
	peraResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	peraResourceDesc.SampleDesc.Count = 1;
	peraResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	peraResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	result = device->CreateCommittedResource(
		&peraHeapProps, 
		D3D12_HEAP_FLAG_NONE, 
		&peraResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, 
		nullptr,
		IID_PPV_ARGS(_peraVB.ReleaseAndGetAddressOf()));

	PeraVertex* mappedPera = nullptr;
	_peraVB->Map(0, nullptr, (void**)&mappedPera);
	std::copy(std::begin(pv), std::end(pv), mappedPera);
	_peraVB->Unmap(0, nullptr);


	// 頂点バッファビューの作成
	_peraVBV.BufferLocation = _peraVB->GetGPUVirtualAddress();
	_peraVBV.SizeInBytes = sizeof(pv);
	_peraVBV.StrideInBytes = sizeof(PeraVertex);


	//ペラポリゴンパイプラインとルートシグネチャ

	D3D12_INPUT_ELEMENT_DESC layout[2] = {
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	gpsDesc.InputLayout.NumElements = _countof(layout);
	gpsDesc.InputLayout.pInputElementDescs = layout;

	ComPtr<ID3DBlob> vs;
	ComPtr<ID3DBlob> ps;
	ComPtr<ID3DBlob> errBlob;	//エラーオブジェクト

	result = D3DCompileFromFile(
		L"peraVertex.hlsl",	//シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//インクルード可能にする
		"main",		//エントリーポイント名
		"vs_5_0",	//シェーダモデル指定
		0,
		0,
		vs.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf()
	);

	//エラーなら
	if (FAILED(result))
	{
		//errorBlobからエラーの内容をstring型にコピー
		std::string error;
		error.resize(errBlob->GetBufferSize());

		std::copy_n(
			(char*)errBlob->GetBufferPointer(),
			errBlob->GetBufferSize(),
			error.begin()
		);
		error += "\n";
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	result = D3DCompileFromFile(
		L"peraPixel.hlsl",	//シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//インクルード可能にする
		"main",		//エントリーポイント名
		"ps_5_0",	//シェーダモデル指定
		0,
		0,
		ps.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf()
	);

	//エラーなら
	if (FAILED(result))
	{
		//errorBlobからエラーの内容をstring型にコピー
		std::string error;
		error.resize(errBlob->GetBufferSize());

		std::copy_n(
			(char*)errBlob->GetBufferPointer(),
			errBlob->GetBufferSize(),
			error.begin()
		);
		error += "\n";
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps.Get());

	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = 0;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> rsBlob;
	//ComPtr<ID3DBlob> errBlob;

	result = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		rsBlob.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf()
		);

	result = device->CreateRootSignature(
		0,
		rsBlob->GetBufferPointer(),
		rsBlob->GetBufferSize(),
		IID_PPV_ARGS(_peraResource.ReleaseAndGetAddressOf())
	);

	//パイプラインステート

	// デスクリプタレンジ
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ

	// スタティックサンプラー
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	// ルートシグネチャの設定
	// ルートパラメータ
	CD3DX12_ROOT_PARAMETER rootparams[3];
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[2].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> rootSigBlob;

	// バージョン自動判定のシリアライズ
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errBlob);
	
	// ルートシグネチャの生成
	result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&_peraRS));
	assert(SUCCEEDED(result));


	gpsDesc.pRootSignature = _peraRS.Get();
	result = device->CreateGraphicsPipelineState(
		&gpsDesc,
		IID_PPV_ARGS(_peraPipeline.ReleaseAndGetAddressOf())
	);

}

#pragma endregion
#pragma region フェンス
void DirectXCommon::InitializeFence()
{
	HRESULT result;
	//フェンスの生成
	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
}
#pragma endregion

#pragma region 描画前処理
void DirectXCommon::PreDraw()
{
	//バックバッファの番号を取得(2つなので0番か1番)
	UINT bbIndex = GetSwapChain()->GetCurrentBackBufferIndex();

	//リソースバリア変更
	barrierDesc.Transition.pResource = backBuffers[bbIndex].Get();	//バックバッファを指定
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;	//表示状態から
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;	//描画状態へ
	GetCommandList()->ResourceBarrier(1, &barrierDesc);

	// 2. 描画先の変更
	// レンダーターゲットビューのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetRtvHeap()->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += bbIndex * GetDevice()->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	GetCommandList()->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

	//1パス目
	auto rtvHeapPointer = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();

	GetCommandList()->OMSetRenderTargets(
		1, &rtvHeapPointer, false,
		&dsvHandle);

	// リソースバリア変更
	barrierDesc.Transition.pResource = backBuffers[bbIndex].Get();	//バックバッファを指定
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//表示状態から
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;	//描画状態へ
	GetCommandList()->ResourceBarrier(1, &barrierDesc);



	////リソースバリア変更
	//barrierDesc.Transition.pResource = _peraResource.Get();	//バックバッファを指定
	//barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;	//表示状態から
	//barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;	//描画状態へ
	//GetCommandList()->ResourceBarrier(1, &barrierDesc);

	////// 2. 描画先の変更
	////// レンダーターゲットビューのハンドルを取得
	////D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetRtvHeap()->GetCPUDescriptorHandleForHeapStart();
	////D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	////
	////1パス目
	//auto rtvHeapPointer = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();

	//GetCommandList()->OMSetRenderTargets(
	//	1, &rtvHeapPointer, false,
	//	&dsvHandle);

	//// リソースバリア変更
	//barrierDesc.Transition.pResource =_peraResource.Get();	//バックバッファを指定
	//barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//表示状態から
	//barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;	//描画状態へ
	//GetCommandList()->ResourceBarrier(1, &barrierDesc);



	// 3. 画面クリアコマンド   R     G    B    A

	FLOAT clearColor[] = { 0.0f,0.0f,0.01f,0.0f };
	GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// ビューポートの設定
	CD3DX12_VIEWPORT viewport;
	viewport.Width = window_width;
	viewport.Height = window_height;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//ビューポート設定コマンドをコマンドリストに積む
	GetCommandList()->RSSetViewports(1, &viewport);
	// シザリング矩形の設定
	CD3DX12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = scissorRect.left + window_width;
	scissorRect.top = 0;
	scissorRect.bottom = scissorRect.top + window_height;
	GetCommandList()->RSSetScissorRects(1, &scissorRect);

}
#pragma endregion 
#pragma region 描画後処理
void DirectXCommon::PostDraw()
{

	commandList->SetGraphicsRootSignature(_peraRS.Get());
	commandList->SetPipelineState(_peraPipeline.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	commandList->IASetVertexBuffers(0, 1, &_peraVBV);
	commandList->DrawInstanced(4, 1, 0, 0);

	HRESULT result;

	// 5. リソースバリアを書き込み禁止に
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//描画状態から
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;			//表示状態へ
	GetCommandList()->ResourceBarrier(1, &barrierDesc);

	//命令のクローズ
	result = GetCommandList()->Close();
	assert(SUCCEEDED(result));
	//コマンドリストの実行
	ID3D12CommandList* commandLists[] = { GetCommandList() };
	GetCommandQueue()->ExecuteCommandLists(1, commandLists);

	//画面に表示するバッファをクリップ
	result = GetSwapChain()->Present(1, 0);
	assert(SUCCEEDED(result));


	//コマンドの実行完了を待つ
	GetCommandQueue()->Signal(GetFence(), ++fenceVal);
	if (GetFence()->GetCompletedValue() != fenceVal)
	{
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		GetFence()->SetEventOnCompletion(fenceVal, event);
		if (event != NULL) {
			WaitForSingleObject(event, INFINITE);
		}
		if (event != NULL) {
			CloseHandle(event);
		}
	}

	//キューをクリア
	result = GetCommandAllocator()->Reset();
	assert(SUCCEEDED(result));
	//再びコマンドリストを貯める準備
	result = GetCommandList()->Reset(GetCommandAllocator(), nullptr);
	assert(SUCCEEDED(result));
}
#pragma endregion
