#include "DirectXCommon.h"

//�V���O���g���C���X�^���X���擾
DirectXCommon* DirectXCommon::GetInstance()
{
	static DirectXCommon instance;
	return &instance;
}

//����������
void DirectXCommon::Initialize(WinApp* winApp)
{
	winApp_ = winApp;
	HRESULT result;

	//�f�o�C�X������
	InitializeDevice();
	//�R�}���h���X�g������
	InitializeCommand();
	//�X���b�v�`�F�[��������
	InitializeSwapchain();
	//�����_�[�^�[�Q�b�g�r���[������
	InitializeRenderTargetView();
	//�[�x�o�b�t�@
	InitializeDepthBuffer();

	InitializeMultipassRendering();

	//�t�F���X����
	InitializeFence();
}

#pragma region �f�o�C�X������
void DirectXCommon::InitializeDevice()
{
	HRESULT result;

#ifdef _DEBUG
	//�f�o�b�O���C���[���I����
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}

#endif

	//DXGI�t�@�N�g���[�̐���
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//�A�_�v�^�[
	//�p�t�H�[�}���X���������̂��珇�ɁA���ׂẴA�_�v�^�[��񋓂���
	for (UINT i = 0;
		dxgiFactory->EnumAdapterByGpuPreference(
			i,
			DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND;
		i++)
	{
		//���I�z��ɒǉ�����
		adapters.push_back(tmpAdapter);
	}

	//�Ó��ȃA�_�v�^�[��I�ʂ���
	for (size_t i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC3 adapterDesc;
		//�A�_�v�^�[�̏����擾����
		adapters[i]->GetDesc3(&adapterDesc);

		//�\�t�g�E�F�A�f�o�C�X�����
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			//�f�o�C�X���̗p���ă��[�v�𔲂���
			tmpAdapter = adapters[i];
			break;
		}
	}

	//�Ή����x���̐���
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
		//�̗p�����A�_�v�^�[���f�o�C�X�Ő���
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i],
			IID_PPV_ARGS(&device));
		if (result == S_OK)
		{
			//�f�o�C�X�����ł������_�Ń��[�v�𔲂���
			featureLevel = levels[i];
			break;
		}
	}
}
#pragma endregion
#pragma region �R�}���h���X�g������
void DirectXCommon::InitializeCommand()
{
	HRESULT result;
	//�R�}���h�A���P�[�^�𐶐�
	result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(result));

	//�R�}���h���X�g�𐶐�
	result = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(result));

	//�R�}���h�L���[�ɐݒ�
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//�R�}���h�L���[�𐶐�
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));
}
#pragma endregion
#pragma region �X���b�v�`�F�[��
void DirectXCommon::InitializeSwapchain()
{
	HRESULT result;
	//�X���b�v�`�F�[���̐ݒ�
	swapChainDesc.Width = window_width;
	swapChainDesc.Height = window_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//�F���̏���
	swapChainDesc.SampleDesc.Count = 1;					//�}���`�T���v�����O���Ȃ�
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;	//�o�b�N�o�b�t�@�p
	swapChainDesc.BufferCount = 2;						//�o�b�t�@����2�ɐݒ�
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//�t���b�v��͔j��
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain1>swapchain1;

	//�X���b�v�`�F�[���̐��� 
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
#pragma region �����_�[�^�[�Q�b�g�r���[ 
void DirectXCommon::InitializeRenderTargetView()
{
	// �f�X�N���v�^�q�[�v�̐ݒ� 
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // �����_�[�^�[�Q�b�g�r���[ 
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount; //���\�̓��

	// �f�X�N���v�^�q�[�v�̐��� 
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	//�o�b�N�o�b�t�@
	backBuffers.resize(swapChainDesc.BufferCount);

	//�X���b�v�`�F�[���̑S�Ẵo�b�t�@�ɂ��ď�������
	for (size_t i = 0; i < backBuffers.size(); i++)
	{
		//�X���b�v�`�F�[������o�b�t�@���擾
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		//�f�X�N���v�^�q�[�v�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		//�����\�ŃA�h���X�������
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//�����_�[�^�[�Q�b�g�r���[�̐ݒ�
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		//�V�F�[�_�[�̌v�Z���ʂ�SRGB�ɕϊ����ď�������
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//�����_�[�^�[�Q�b�g�r���̐���
		device->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, rtvHandle);
	}

	////�}���`�p�X�����^�����O

	////�쐬�ς݂̃q�[�v�����g���Ă����ꖇ���
	//auto heapDesc = rtvHeapDesc;

	////�g���Ă���o�b�t�@�[�̏��𗘗p����
	//auto& bbuff = backBuffers[0];
	//auto resDesc = bbuff->GetDesc();

	//D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	////�����^�����O���̃N���A���Ɠ����l
	//FLOAT clsClr[] = { 0.0f,0.0f,0.01f,0.0f };
	//D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);

	//auto result = device->CreateCommittedResource(&heapProp,
	//	D3D12_HEAP_FLAG_NONE,
	//	&resDesc,
	//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	//	&clearValue,
	//	IID_PPV_ARGS(_peraResource.ReleaseAndGetAddressOf()));

	////�r���[�����

	//ComPtr<ID3D12DescriptorHeap> _peraRTVHeap; // �����_�[�^�[�Q�b�g�p
	//ComPtr<ID3D12DescriptorHeap> _peraSRVHeap; //�e�N�X�`���p

	//// �f�X�N���v�^�q�[�v�̐ݒ�
	//srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // �V�F�[�_�[���\�[�X�r���[ 
	//srvHeapDesc.NumDescriptors = swapChainDesc.BufferCount; //���\�̓��

	//// �f�X�N���v�^�q�[�v�̐��� 
	//device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_peraRTVHeap));
	//device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&_peraSRVHeap));

	////RTV�p�q�[�v�����
	//heapDesc.NumDescriptors = 1;
	//result = device->CreateDescriptorHeap(
	//	&heapDesc,
	//	IID_PPV_ARGS(_peraResource.ReleaseAndGetAddressOf())
	//);

	//D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	//rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	//rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	////�����_�[�^�[�Q�b�g�r���[�����

	//device->CreateRenderTargetView(
	//	_peraResource.Get(),
	//	&rtvDesc,
	//	_peraRTVHeap->GetCPUDescriptorHandleForHeapStart());

	////SRV�p�q�[�v�����
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

	////�V�F�[�_�[���\�[�X�r���[�����

	//device->CreateShaderResourceView(
	//	_peraResource.Get(),
	//	&srvDesc,
	//	_peraSRVHeap->GetCPUDescriptorHandleForHeapStart()
	//);

	////1�p�X��
	//auto rtvHeapPointer = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();

	//commandList->OMSetRenderTargets(
	//	1, &rtvHeapPointer, false,
	//	&dsvHeap->GetCPUDescriptorHandleForHeapStart());
}
#pragma endregion
#pragma region �[�x�o�b�t�@
void DirectXCommon::InitializeDepthBuffer()
{
	HRESULT result;

	//���\�[�X�ݒ�
	depthResorceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResorceDesc.Width = window_width;	//�����_�[�^�[�Q�b�g�ɍ��킹��
	depthResorceDesc.Height = window_height;	//�����_�[�^�[�Q�b�g�ɍ��킹��
	depthResorceDesc.DepthOrArraySize = 1;
	depthResorceDesc.Format = DXGI_FORMAT_D32_FLOAT;	//�[�x�l�t�H�[�}�b�g
	depthResorceDesc.SampleDesc.Count = 1;
	depthResorceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//�f�v�X�X�e���V��

	//�k�x�l�p�q�[�v�v���p�e�B
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	//�[�x�l�̃N���A�ݒ�
	depthClearValue.DepthStencil.Depth = 1.0f;	//�[�x�l1.0f(�ő�l)�ŃN���A
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;	//�[�x�l�t�H�[�}�b�g

	//���\�[�X����
	result = GetDevice()->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResorceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,	//�[�x�l�������݂Ɏg�p
		&depthClearValue,
		IID_PPV_ARGS(&depthBuff)
	);

	//�[�x�r���[�p�f�X�N���v�^�q�[�v�쐬
	dsvHeapDesc.NumDescriptors = 1;	//�[�x�r���[��1��
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//�f�v�X�X�e���V���r���[
	result = GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));

	//�[�x�X�e���V���r���[�̐���
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;	//�[�x�l�t�H�[�}�b�g
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
	
	//�}���`�p�X�����^�����O

	//�쐬�ς݂̃q�[�v�����g���Ă����ꖇ���
	auto heapDesc = rtvHeapDesc;

	//�g���Ă���o�b�t�@�[�̏��𗘗p����
	auto& bbuff = backBuffers[0];
	auto resDesc = bbuff->GetDesc();

	D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	//�����^�����O���̃N���A���Ɠ����l
	FLOAT clsClr[] = { 0.0f,0.0f,0.01f,0.0f };
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);

	result = device->CreateCommittedResource(&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(_peraResource.ReleaseAndGetAddressOf()));

	//�r���[�����

	// �f�X�N���v�^�q�[�v�̐ݒ�
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // �V�F�[�_�[���\�[�X�r���[ 
	srvHeapDesc.NumDescriptors = swapChainDesc.BufferCount; //���\�̓��

	// �f�X�N���v�^�q�[�v�̐��� 
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_peraRTVHeap));
	device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&_peraSRVHeap));

	//RTV�p�q�[�v�����
	heapDesc.NumDescriptors = 1;
	result = device->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(_peraResource.ReleaseAndGetAddressOf())
	);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//�����_�[�^�[�Q�b�g�r���[�����

	device->CreateRenderTargetView(
		_peraResource.Get(),
		&rtvDesc,
		_peraRTVHeap->GetCPUDescriptorHandleForHeapStart());

	//SRV�p�q�[�v�����
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

	//�V�F�[�_�[���\�[�X�r���[�����

	device->CreateShaderResourceView(
		_peraResource.Get(),
		&srvDesc,
		_peraSRVHeap->GetCPUDescriptorHandleForHeapStart()
	);

	//�y���|���S������

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

	// ���\�[�X�ݒ�
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


	// ���_�o�b�t�@�r���[�̍쐬
	_peraVBV.BufferLocation = _peraVB->GetGPUVirtualAddress();
	_peraVBV.SizeInBytes = sizeof(pv);
	_peraVBV.StrideInBytes = sizeof(PeraVertex);


	//�y���|���S���p�C�v���C���ƃ��[�g�V�O�l�`��

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
	ComPtr<ID3DBlob> errBlob;	//�G���[�I�u�W�F�N�g

	result = D3DCompileFromFile(
		L"peraVertex.hlsl",	//�V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//�C���N���[�h�\�ɂ���
		"main",		//�G���g���[�|�C���g��
		"vs_5_0",	//�V�F�[�_���f���w��
		0,
		0,
		vs.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf()
	);

	//�G���[�Ȃ�
	if (FAILED(result))
	{
		//errorBlob����G���[�̓��e��string�^�ɃR�s�[
		std::string error;
		error.resize(errBlob->GetBufferSize());

		std::copy_n(
			(char*)errBlob->GetBufferPointer(),
			errBlob->GetBufferSize(),
			error.begin()
		);
		error += "\n";
		//�G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	result = D3DCompileFromFile(
		L"peraPixel.hlsl",	//�V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//�C���N���[�h�\�ɂ���
		"main",		//�G���g���[�|�C���g��
		"ps_5_0",	//�V�F�[�_���f���w��
		0,
		0,
		ps.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf()
	);

	//�G���[�Ȃ�
	if (FAILED(result))
	{
		//errorBlob����G���[�̓��e��string�^�ɃR�s�[
		std::string error;
		error.resize(errBlob->GetBufferSize());

		std::copy_n(
			(char*)errBlob->GetBufferPointer(),
			errBlob->GetBufferSize(),
			error.begin()
		);
		error += "\n";
		//�G���[���e���o�̓E�B���h�E�ɕ\��
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

	//�p�C�v���C���X�e�[�g

	// �f�X�N���v�^�����W
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 ���W�X�^

	// �X�^�e�B�b�N�T���v���[
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	// ���[�g�V�O�l�`���̐ݒ�
	// ���[�g�p�����[�^
	CD3DX12_ROOT_PARAMETER rootparams[3];
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[2].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> rootSigBlob;

	// �o�[�W������������̃V���A���C�Y
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errBlob);
	
	// ���[�g�V�O�l�`���̐���
	result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&_peraRS));
	assert(SUCCEEDED(result));


	gpsDesc.pRootSignature = _peraRS.Get();
	result = device->CreateGraphicsPipelineState(
		&gpsDesc,
		IID_PPV_ARGS(_peraPipeline.ReleaseAndGetAddressOf())
	);

}

#pragma endregion
#pragma region �t�F���X
void DirectXCommon::InitializeFence()
{
	HRESULT result;
	//�t�F���X�̐���
	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
}
#pragma endregion

#pragma region �`��O����
void DirectXCommon::PreDraw()
{
	//�o�b�N�o�b�t�@�̔ԍ����擾(2�Ȃ̂�0�Ԃ�1��)
	UINT bbIndex = GetSwapChain()->GetCurrentBackBufferIndex();

	//���\�[�X�o���A�ύX
	barrierDesc.Transition.pResource = backBuffers[bbIndex].Get();	//�o�b�N�o�b�t�@���w��
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;	//�\����Ԃ���
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;	//�`���Ԃ�
	GetCommandList()->ResourceBarrier(1, &barrierDesc);

	// 2. �`���̕ύX
	// �����_�[�^�[�Q�b�g�r���[�̃n���h�����擾
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetRtvHeap()->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += bbIndex * GetDevice()->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	GetCommandList()->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

	//1�p�X��
	auto rtvHeapPointer = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();

	GetCommandList()->OMSetRenderTargets(
		1, &rtvHeapPointer, false,
		&dsvHandle);

	// ���\�[�X�o���A�ύX
	barrierDesc.Transition.pResource = backBuffers[bbIndex].Get();	//�o�b�N�o�b�t�@���w��
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//�\����Ԃ���
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;	//�`���Ԃ�
	GetCommandList()->ResourceBarrier(1, &barrierDesc);



	////���\�[�X�o���A�ύX
	//barrierDesc.Transition.pResource = _peraResource.Get();	//�o�b�N�o�b�t�@���w��
	//barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;	//�\����Ԃ���
	//barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;	//�`���Ԃ�
	//GetCommandList()->ResourceBarrier(1, &barrierDesc);

	////// 2. �`���̕ύX
	////// �����_�[�^�[�Q�b�g�r���[�̃n���h�����擾
	////D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetRtvHeap()->GetCPUDescriptorHandleForHeapStart();
	////D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	////
	////1�p�X��
	//auto rtvHeapPointer = _peraRTVHeap->GetCPUDescriptorHandleForHeapStart();

	//GetCommandList()->OMSetRenderTargets(
	//	1, &rtvHeapPointer, false,
	//	&dsvHandle);

	//// ���\�[�X�o���A�ύX
	//barrierDesc.Transition.pResource =_peraResource.Get();	//�o�b�N�o�b�t�@���w��
	//barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//�\����Ԃ���
	//barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;	//�`���Ԃ�
	//GetCommandList()->ResourceBarrier(1, &barrierDesc);



	// 3. ��ʃN���A�R�}���h   R     G    B    A

	FLOAT clearColor[] = { 0.0f,0.0f,0.01f,0.0f };
	GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// �r���[�|�[�g�̐ݒ�
	CD3DX12_VIEWPORT viewport;
	viewport.Width = window_width;
	viewport.Height = window_height;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//�r���[�|�[�g�ݒ�R�}���h���R�}���h���X�g�ɐς�
	GetCommandList()->RSSetViewports(1, &viewport);
	// �V�U�����O��`�̐ݒ�
	CD3DX12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = scissorRect.left + window_width;
	scissorRect.top = 0;
	scissorRect.bottom = scissorRect.top + window_height;
	GetCommandList()->RSSetScissorRects(1, &scissorRect);

}
#pragma endregion 
#pragma region �`��㏈��
void DirectXCommon::PostDraw()
{

	commandList->SetGraphicsRootSignature(_peraRS.Get());
	commandList->SetPipelineState(_peraPipeline.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	commandList->IASetVertexBuffers(0, 1, &_peraVBV);
	commandList->DrawInstanced(4, 1, 0, 0);

	HRESULT result;

	// 5. ���\�[�X�o���A���������݋֎~��
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;	//�`���Ԃ���
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;			//�\����Ԃ�
	GetCommandList()->ResourceBarrier(1, &barrierDesc);

	//���߂̃N���[�Y
	result = GetCommandList()->Close();
	assert(SUCCEEDED(result));
	//�R�}���h���X�g�̎��s
	ID3D12CommandList* commandLists[] = { GetCommandList() };
	GetCommandQueue()->ExecuteCommandLists(1, commandLists);

	//��ʂɕ\������o�b�t�@���N���b�v
	result = GetSwapChain()->Present(1, 0);
	assert(SUCCEEDED(result));


	//�R�}���h�̎��s������҂�
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

	//�L���[���N���A
	result = GetCommandAllocator()->Reset();
	assert(SUCCEEDED(result));
	//�ĂуR�}���h���X�g�𒙂߂鏀��
	result = GetCommandList()->Reset(GetCommandAllocator(), nullptr);
	assert(SUCCEEDED(result));
}
#pragma endregion
