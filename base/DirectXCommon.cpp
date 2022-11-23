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
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//レンダーターゲットビュの生成
		device->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, rtvHandle);
	}

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
	//作成済みのヒープ情報を使ってもう一枚作る
	auto heapDesc = rtvHeapDesc;

	//マルチパスレンタリング

	////使っているバッファーの情報を利用する
	//auto& bbuff = backBuffers[0];
	//auto resDesc = bbuff->GetDesc();

	//D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	//{
	//	//レンタリング時のクリア時と同じ値
	//	float clsClr[] = { 100.5f,0.5f,0.5f,1.0f };
	//	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);

	//	auto result = device->CreateCommittedResource(&heapProp,
	//		D3D12_HEAP_FLAG_NONE,
	//		&resDesc,
	//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	//		&clearValue,
	//		IID_PPV_ARGS(_peraResource.ReleaseAndGetAddressOf()));
	//}

	////ビューを作る

	////RTV用ヒープを作る
	//heapDesc.NumDescriptors = 1;
	//result = device->CreateDescriptorHeap(
	//	&heapDesc,
	//	IID_PPV_ARGS(_peraRTVHeap.ReleaseAndGetAddressOf())
	//);

	//D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	//rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	//rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//auto handle = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();

	////レンダーターゲットビューを作る
	//device->CreateRenderTargetView(
	//	_peraResource.Get(),
	//	&rtvDesc,
	//	handle);

	//--------RTV--------

	// デスクリプタヒープの設定 
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー 
	rtvHeapDesc.NumDescriptors = 1; //裏表の二つ

	// デスクリプタヒープの生成 
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&_peraRTVHeap));

	//スワップチェーンの全てのバッファについて処理する

	//スワップチェーンからバッファを取得
	swapChain->GetBuffer((UINT)0, IID_PPV_ARGS(&_peraResource));
	//デスクリプタヒープのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();
	//裏か表でアドレスがずれる
	rtvHandle.ptr += 0 * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
	//レンダーターゲットビューの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//シェーダーの計算結果をSRGBに変換して書き込む
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	
	//レンダーターゲットビュの生成
	device->CreateRenderTargetView(_peraResource.Get(), &rtvDesc, rtvHandle);


	////SRV用ヒープを作る
	//heapDesc.NumDescriptors = 1;
	//heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	//result = device->CreateDescriptorHeap(
	//	&heapDesc,
	//	IID_PPV_ARGS(_peraSRVHeap.ReleaseAndGetAddressOf())
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

	//--------SRV--------

	// デスクリプタヒープを生成	
	D3D12_DESCRIPTOR_HEAP_DESC srvdescHeap = {};
	srvdescHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvdescHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
	srvdescHeap.NumDescriptors = 1; // シェーダーリソースビュー1つ
	result = device->CreateDescriptorHeap(&srvdescHeap, IID_PPV_ARGS(&_peraSRVHeap));//生成
	if (FAILED(result)) {
		assert(0);
	}
	// デスクリプタサイズを取得
	descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// シェーダリソースビュー作成
	cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(_peraSRVHeap->GetCPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);
	gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(_peraSRVHeap->GetGPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);

	//使っているバッファーの情報を利用する
	auto& bbuff = backBuffers[0];

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体
	D3D12_RESOURCE_DESC resDesc = bbuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(_peraResource.Get(), //ビューと関連付けるバッファ
		&srvDesc, //テクスチャ設定情報
		cpuDescHandleSRV
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

	// リソースバリア変更
	barrierDesc.Transition.pResource = backBuffers[bbIndex].Get();	//バックバッファを指定
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//表示状態から
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;	//描画状態へ
	GetCommandList()->ResourceBarrier(1, &barrierDesc);

	// 2. 描画先の変更
	// レンダーターゲットビューのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetRtvHeap()->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += bbIndex * GetDevice()->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	GetCommandList()->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);


	// 3. 画面クリアコマンド   R     G    B    A

	FLOAT clearColor[] = {0.5f,0.5f,100.5f,1.0f };
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

	// 5. リソースバリアを書き込み禁止に
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;	//描画状態から
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;			//表示状態へ
	GetCommandList()->ResourceBarrier(1, &barrierDesc);

	HRESULT result;

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
void DirectXCommon::PeraPreDraw()
{

	//リソースバリア変更
	barrierDesc.Transition.pResource = _peraResource.Get();	//バックバッファを指定
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;	//表示状態から
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;	//描画状態へ
	GetCommandList()->ResourceBarrier(1, &barrierDesc);

	// 2. 描画先の変更
	// レンダーターゲットビューのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	//1パス目
	auto rtvHeapPointer = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();

	// 3. 画面クリアコマンド   R     G    B    A

	FLOAT clearColor[] = { 0.5f,0.5f,100.5f,1.0f };
	GetCommandList()->ClearRenderTargetView(rtvHeapPointer, clearColor, 0, nullptr);
	GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	
	GetCommandList()->OMSetRenderTargets(
		1, &rtvHeapPointer, false,
		&dsvHandle);

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
void DirectXCommon::PeraPostDraw()
{



	// 5. リソースバリアを書き込み禁止に
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//描画状態から
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;			//表示状態へ
	GetCommandList()->ResourceBarrier(1, &barrierDesc);


	HRESULT result;

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
