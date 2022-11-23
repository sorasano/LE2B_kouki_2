#include "Pera.h"

//�y���|���S������

void Pera::Initialize(ComPtr<ID3D12Device> device, DirectXCommon* dx) {

	this->dx = dx;

	HRESULT result;

	// ���\�[�X�ݒ�
	CD3DX12_HEAP_PROPERTIES peraHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	CD3DX12_RESOURCE_DESC peraResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(pv));

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


	// �q�[�v�v���p�e�B
	CD3DX12_HEAP_PROPERTIES heapPropsConstantBuffer = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// ���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC resourceDescConstantBuffer =
		CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff);

	// �萔�o�b�t�@�̐���
	result = device->CreateCommittedResource(
		&heapPropsConstantBuffer, // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resourceDescConstantBuffer, // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));
	assert(SUCCEEDED(result));

	// �萔�o�b�t�@�Ƀf�[�^�]��
	ConstBufferData* constMap = nullptr;
	result = constBuff->Map(0, nullptr, (void**)&constMap); // �}�b�s���O
	constMap->mode = mode;
	constMap->time = time;
	constBuff->Unmap(0, nullptr);

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
			DXGI_FORMAT_R32G32_FLOAT,
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

	{
		//D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
		//rsDesc.NumParameters = 0;
		//rsDesc.NumStaticSamplers = 0;
		//rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		//D3D12_DESCRIPTOR_RANGE range = {};

		//range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		//range.BaseShaderRegister = 0;
		//range.NumDescriptors = 1;

		//D3D12_ROOT_PARAMETER rp = {};
		//rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		//rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		//rp.DescriptorTable.pDescriptorRanges = &range;
		//rp.DescriptorTable.NumDescriptorRanges = 1;

		//D3D12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(0);

		//rsDesc.NumParameters = 1;
		//rsDesc.pParameters = &rp;
		//rsDesc.NumStaticSamplers = 1;
		//rsDesc.pStaticSamplers = &sampler;

		CD3DX12_DESCRIPTOR_RANGE descriptorRange{};
		descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParams[2];
		rootParams[0].InitAsConstantBufferView(0);
		rootParams[1].InitAsDescriptorTable(1, &descriptorRange, D3D12_SHADER_VISIBILITY_ALL);

		// �e�N�X�`���T���v���[�̐ݒ�
		D3D12_STATIC_SAMPLER_DESC samplerDesc{};
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //���J��Ԃ��i�^�C�����O�j
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //�c�J��Ԃ��i�^�C�����O�j
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //���s�J��Ԃ��i�^�C�����O�j
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;  //�{�[�_�[�̎��͍�
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;                   //�S�ă��j�A���
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;                                 //�~�b�v�}�b�v�ő�l
		samplerDesc.MinLOD = 0.0f;                                              //�~�b�v�}�b�v�ŏ��l
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           //�s�N�Z���V�F�[�_����̂ݎg�p�\

		// ���[�g�V�O�l�`���̐ݒ�
		D3D12_ROOT_SIGNATURE_DESC rsDesc{};
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rsDesc.pParameters = rootParams; //���[�g�p�����[�^�̐擪�A�h���X
		rsDesc.NumParameters = _countof(rootParams);        //���[�g�p�����[�^��

		rsDesc.pStaticSamplers = &samplerDesc;
		rsDesc.NumStaticSamplers = 1;


		ComPtr<ID3DBlob> rsBlob;
		auto result = D3D12SerializeRootSignature(
			&rsDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			rsBlob.ReleaseAndGetAddressOf(),
			errBlob.ReleaseAndGetAddressOf()
		);

		result = device->CreateRootSignature(
			0,
			rsBlob->GetBufferPointer(),
			rsBlob->GetBufferSize(),
			IID_PPV_ARGS(_peraRS.ReleaseAndGetAddressOf())
		);

		gpsDesc.pRootSignature = _peraRS.Get();
		result = device->CreateGraphicsPipelineState(
			&gpsDesc,
			IID_PPV_ARGS(_peraPipeline.ReleaseAndGetAddressOf())
		);
	}

}

void Pera::Update()
{

	time += 0.001;

	// �萔�o�b�t�@�Ƀf�[�^�]��
	ConstBufferData* constMap = nullptr;
	HRESULT result = constBuff->Map(0, nullptr, (void**)&constMap); // �}�b�s���O
	constMap->mode = mode;
	constMap->time = time;
	constBuff->Unmap(0, nullptr);

}

void Pera::Draw() {

	this->commandList = dx->GetCommandList();


	commandList->SetGraphicsRootSignature(_peraRS.Get());
	commandList->SetPipelineState(_peraPipeline.Get());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	commandList->IASetVertexBuffers(0, 1, &_peraVBV);

	// �萔�o�b�t�@(CBV)�̐ݒ�R�}���h
	commandList->SetGraphicsRootConstantBufferView(0, constBuff->GetGPUVirtualAddress());


	this->_peraSRVHeap = dx->GetPeraSRVHeap();

	commandList->SetDescriptorHeaps(1, _peraSRVHeap.GetAddressOf());

	auto handle = _peraSRVHeap->GetGPUDescriptorHandleForHeapStart();

	commandList->SetGraphicsRootDescriptorTable(1, handle);


	commandList->DrawInstanced(4, 1, 0, 0);

}