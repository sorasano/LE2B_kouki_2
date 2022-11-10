#include "Cube.h"
#define PI 3.141592653589793238462643

using namespace DirectX;
using namespace Microsoft::WRL;


Cube* Cube::GetInstance()
{
	static Cube instance;
	return &instance; 
}

void Cube::Initialize(XMFLOAT3 size, DirectXCommon* dx_)
{
	dx = dx_;
	HRESULT result;

	vertices.resize(verticesCount);
	indices.resize(indicesCount);

	//���_������
	InitializeVertex(size);
	//�C���f�b�N�X�o�b�t�@������
	InitializeIndexBuff();
	//���_�o�b�t�@������
	InitializeVertBuff();
	//�V�F�[�_�ǂݍ���
	CompileShader(L"BasicVS.hlsl", L"BasicPS.hlsl");
	//���[�g�V�O�l�`��
	InitializeRootSignature();
	//�p�C�v���C��
	InitializePipeline();
	//�p�C�v���C���X�e�[�g
	InitializePipelineState();


	//�萔�o�b�t�@
	//�q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;	//GPU�̓]���p
	//���\�[�X�ݒ�
	D3D12_RESOURCE_DESC cbResourceDesc{};
	cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbResourceDesc.Width = (sizeof(ConstBufferDataMaterial) + 0xff) & ~0xff;	//256�o�C�g�A���C�������g
	cbResourceDesc.Height = 1;
	cbResourceDesc.DepthOrArraySize = 1;
	cbResourceDesc.MipLevels = 1;
	cbResourceDesc.SampleDesc.Count = 1;
	cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//�萔�o�b�t�@�̐���
	result = dx->GetDevice()->CreateCommittedResource(
		&cbHeapProp,	//�q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&cbResourceDesc,	//���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffMaterial)
	);
	assert(SUCCEEDED(result));

	//�萔�o�b�t�@�̃}�b�s���O
	ConstBufferDataMaterial* constMapMaterial = nullptr;
	result = constBuffMaterial->Map(0, nullptr, (void**)&constMapMaterial);	//�}�b�s���O
	assert(SUCCEEDED(result));
	//�l���������ނƎ����I�ɓ]�������
	constMapMaterial->color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}

#pragma region ���_������
void Cube::InitializeVertex(XMFLOAT3 size)
{
	//���_�f�[�^
	Vertex v[] = {
		//�O
		{{-size.x / 2,-size.y / 2,-size.z / 2},{},{0.0f,1.0f} },	//0
		{{-size.x / 2, size.y / 2,-size.z / 2},{},{0.0f,0.0f} },	//1 
		{{ size.x / 2,-size.y / 2,-size.z / 2},{},{1.0f,1.0f} },	//2 
		{{ size.x / 2, size.y / 2,-size.z / 2},{},{1.0f,0.0f} },	//3
		//��				 	   
		{{ size.x / 2,-size.y / 2, size.z / 2},{},{0.0f,1.0f} },	//4
		{{ size.x / 2, size.y / 2, size.z / 2},{},{0.0f,0.0f} },	//5
		{{-size.x / 2,-size.y / 2, size.z / 2},{},{1.0f,1.0f} },	//6
		{{-size.x / 2, size.y / 2, size.z / 2},{},{1.0f,0.0f} },	//7
		//��				 	    
		{{-size.x / 2,-size.y / 2,-size.z / 2},{},{0.0f,1.0f} },	//8
		{{-size.x / 2,-size.y / 2, size.z / 2},{},{0.0f,0.0f} },	//9
		{{-size.x / 2, size.y / 2,-size.z / 2},{},{1.0f,1.0f} },	//10
		{{-size.x / 2, size.y / 2, size.z / 2},{},{1.0f,0.0f} },	//11
		//�E				 	    
		{{ size.x / 2,-size.y / 2,-size.z / 2},{},{0.0f,1.0f} },	//12
		{{ size.x / 2,-size.y / 2, size.z / 2},{},{0.0f,0.0f} },	//13
		{{ size.x / 2, size.y / 2,-size.z / 2},{},{1.0f,1.0f} },	//14
		{{ size.x / 2, size.y / 2, size.z / 2},{},{1.0f,0.0f} },	//15
		//��					  	
		{{-size.x / 2,-size.y / 2, size.z / 2},{},{0.0f,1.0f} },	//16
		{{-size.x / 2,-size.y / 2,-size.z / 2},{},{0.0f,0.0f} },	//17
		{{ size.x / 2,-size.y / 2, size.z / 2},{},{1.0f,1.0f} },	//18
		{{ size.x / 2,-size.y / 2,-size.z / 2},{},{1.0f,0.0f} },	//19
		//��				 	    
		{{-size.x / 2, size.y / 2,-size.z / 2},{},{0.0f,1.0f} },	//20
		{{-size.x / 2, size.y / 2, size.z / 2},{},{0.0f,0.0f} },	//21
		{{ size.x / 2, size.y / 2,-size.z / 2},{},{1.0f,1.0f} },	//22
		{{ size.x / 2, size.y / 2, size.z / 2},{},{1.0f,0.0f} },	//23
	};
	//�C���f�b�N�X�f�[�^
	unsigned short in[] =
	{

		//�O
		0,1,2,	//�O�p�`1��
		2,1,3,	//�O�p�`2��
		//��
		4,5,6,
		6,5,7,
		//��
		8,9,10,
		10,9,11,
		//�E
		12,13,14,
		14,13,15,
		//��
		16,17,18,
		18,17,19,
		//��
		20,21,22,
		22,21,23,
	};

	//���_���W�Auv���W�A�C���f�b�N�X�f�[�^����
	for (int i = 0; i < 24; i++)
	{
		vertices[i] = v[i];
	}

	for (int i = 0; i < 36; i++)
	{
		indices[i] = in[i];
	}

	//�@���̌v�Z
	for (int i = 0; i < indices.size() / 3; i++)
	{//�O�p�`1���ƂɌv�Z���Ă���
		//�O�p�`�̃C���f�b�N�X�����o���āA�ꎞ�I�ȕϐ��ɓ����
		unsigned short indices0 = indices[i * 3 + 0];
		unsigned short indices1 = indices[i * 3 + 1];
		unsigned short indices2 = indices[i * 3 + 2];
		//�O�p�`���\�����钸�_���W���x�N�g���ɑ��
		XMVECTOR p0 = XMLoadFloat3(&vertices[indices0].pos);
		XMVECTOR p1 = XMLoadFloat3(&vertices[indices1].pos);
		XMVECTOR p2 = XMLoadFloat3(&vertices[indices2].pos);
		//p0��p1�x�N�g���Ap0��p2�x�N�g�����v�Z�@(�x�N�g���̌��Z)
		XMVECTOR v1 = XMVectorSubtract(p1, p0);
		XMVECTOR v2 = XMVectorSubtract(p2, p0);
		//�O�ς͗������琂���ȃx�N�g��
		XMVECTOR normal = XMVector3Cross(v1, v2);
		//���K��
		normal = XMVector3Normalize(normal);
		//���߂��@���𒸓_�f�[�^�ɑ��
		XMStoreFloat3(&vertices[indices0].normalize, normal);
		XMStoreFloat3(&vertices[indices1].normalize, normal);
		XMStoreFloat3(&vertices[indices2].normalize, normal);
	}

	sizeVB = static_cast<UINT>(sizeof(vertices[0]) * vertices.size());
	sizeIB = static_cast<UINT>(sizeof(uint16_t) * indices.size());

	//���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayout_[] =
	{
		{	//xyz���W
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
#pragma region �C���f�b�N�X�o�b�t�@������
void Cube::InitializeIndexBuff()
{
	HRESULT result;

	D3D12_HEAP_PROPERTIES heapProp{};	//�q�[�v�ݒ�
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;	//CPU�ւ̓]���p
	D3D12_RESOURCE_DESC resDesc{};
	//���\�[�X�ݒ�
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeIB;	//�C���f�b�N�X��񂪓��镪�̃T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	result = dx->GetDevice()->CreateCommittedResource(
		&heapProp,	//�q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resDesc,	//���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff)
	);

	//�C���f�b�N�X�o�b�t�@���}�b�s���O
	uint16_t* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	//�S�C���f�b�N�X�ɑ΂���
	for (int i = 0; i < indices.size(); i++)
	{
		indexMap[i] = indices[i];	//�C���f�b�N�X���R�s�[
	}
	//�}�b�s���O����
	indexBuff->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�r���[�̍쐬
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;
}
#pragma endregion
#pragma region ���_�o�b�t�@������
void Cube::InitializeVertBuff()
{
	HRESULT result;

	//���_�o�b�t�@�̐ݒ�
	D3D12_HEAP_PROPERTIES heapProp{};	//�q�[�v�ݒ�
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;	//CPU�ւ̓]���p
	//���\�[�X�ݒ�
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeVB;	//���_�f�[�^�S�̂̃T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	result = dx->GetDevice()->CreateCommittedResource(
		&heapProp,	//�q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resDesc,	//���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff)
	);
	assert(SUCCEEDED(result));

	//���_�o�b�t�@�ւ̃f�[�^�]��
	//GPU��̃o�b�t�@�ɑΉ��������z�������i���C����������j���擾
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));
	//�S���_�ɑ΂���
	for (int i = 0; i < vertices.size(); i++)
	{
		vertMap[i] = vertices[i];	//���W���R�s�[
	}
	//�Ȃ��������
	vertBuff->Unmap(0, nullptr);

	//���_�o�b�t�@�r���[�̍쐬
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]);
}
#pragma endregion
#pragma region �V�F�[�_
void Cube::CompileShader(const wchar_t* file, const wchar_t* file2)
{
	ID3DBlob* vsBlob_ = nullptr;	//���_�V�F�[�_�[�I�u�W�F�N�g
	ID3DBlob* errorBlob_ = nullptr;	//�G���[�I�u�W�F�N�g

	HRESULT result;

	//���_�V�F�[�_�[�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		file,	//�V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//�C���N���[�h�\�ɂ���
		"main",	//�G���g���[��
		"vs_5_0",	//�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//�f�o�b�O�p�ݒ�
		0,
		&vsBlob_,
		&errorBlob_
	);

	//�G���[�Ȃ�
	if (FAILED(result))
	{
		//errorBlob����G���[�̓��e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob_->GetBufferSize());

		std::copy_n(
			(char*)errorBlob_->GetBufferPointer(),
			errorBlob_->GetBufferSize(),
			error.begin()
		);
		error += "\n";
		//�G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	vsBlob = vsBlob_;
	errorBlob = errorBlob_;

	ID3DBlob* psBlob_ = nullptr;	//���_�V�F�[�_�[�I�u�W�F�N�g

	//�s�N�Z���V�F�[�_�[�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		file2,	//�V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//�C���N���[�h�\�ɂ���
		"main",		//�G���g���[�|�C���g��
		"ps_5_0",	//�V�F�[�_���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//�f�o�b�O�p�ݒ�
		0,
		&psBlob_,
		&errorBlob_
	);

	//�G���[�Ȃ�
	if (FAILED(result))
	{
		//errorBlob����G���[�̓��e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob_->GetBufferSize());

		std::copy_n(
			(char*)errorBlob_->GetBufferPointer(),
			errorBlob_->GetBufferSize(),
			error.begin()
		);
		error += "\n";
		//�G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	psBlob = psBlob_;
	errorBlob = errorBlob_;
}
#pragma endregion
#pragma region ���[�g�V�O�l�`��
void Cube::InitializeRootSignature()
{
	HRESULT result;

	//�f�X�N���v�^�����W�̐ݒ�
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.NumDescriptors = 1;	//��x�̕`��Ɏg���e�N�X�`����1���Ȃ̂�1
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//���[�g�p�����[�^�̐ݒ�
	D3D12_ROOT_PARAMETER rootParams[3] = {};
	//�萔�o�b�t�@0��
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//���
	rootParams[0].Descriptor.ShaderRegister = 0;					//�萔�o�b�t�@�ԍ�
	rootParams[0].Descriptor.RegisterSpace = 0;						//�f�t�H���g�l
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	//�S�ẴV�F�[�_���猩����
	//�e�N�X�`�����W�X�^0��
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//���
	rootParams[1].DescriptorTable.pDescriptorRanges = &descriptorRange;			//�f�X�N���v�^�����W
	rootParams[1].DescriptorTable.NumDescriptorRanges = 1;						//�f�X�N���v�^�����W��
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;				//�S�ẴV�F�[�_���猩����
	//�萔�o�b�t�@1��
	rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	//���
	rootParams[2].Descriptor.ShaderRegister = 1;					//�萔�o�b�t�@�ԍ�
	rootParams[2].Descriptor.RegisterSpace = 0;						//�f�t�H���g�l
	rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	//�S�ẴV�F�[�_�[���猩����
	//�e�N�X�`���T���v���[�̐ݒ�
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//���J��Ԃ�
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//���J��Ԃ�
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		//���s�J��Ԃ�
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;	//�{�[�_�[�̎��͍�
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;					//�S�ă��j�A���
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;						//�~�b�v�}�b�v�ő�l
	samplerDesc.MinLOD = 0.0f;									//�~�b�v�}�b�v�ŏ��l
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//�s�N�Z���V�F�[�_����̂ݎg�p�\

	//���[�g�V�O�l�`���̐ݒ�
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParams;	//���[�g�p�����[�^�̐擪�A�h���X
	rootSignatureDesc.NumParameters = _countof(rootParams);		//���[�g�p�����[�^��
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	//���[�g�V�O�l�`���̃V���A���C�Y
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
#pragma region �p�C�v���C��
void Cube::InitializePipeline()
{
	HRESULT result;

	//�V�F�[�_���p�C�v���C���ɐݒ�
	pipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	//�T���v���}�X�N�̐ݒ�
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;	//�W���ݒ�

	//���X�^���C�U�̐ݒ�
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;	//�J�����O���Ȃ�
	//pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;	//�w�ʂ��J�����O
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;	//�|���S���h��Ԃ�
	pipelineDesc.RasterizerState.DepthClipEnable = true;	//�[�x�N���b�s���O��L����

	//�u�����h�X�e�[�g��L����
	pipelineDesc.BlendState.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;	//RGBA�S�Ẵ`�����l����`��

	//���_���C�A�E�g�̐ݒ�
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);

	//�}�`�̌`��ݒ�
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	//���̑��̐ݒ�
	pipelineDesc.NumRenderTargets = 1;	//�`��Ώۂ̐�
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//0~255�w���RGBA
	pipelineDesc.SampleDesc.Count = 1;	//1�s�N�Z���ɂ�1��T���v�����O

	//�f�v�X�X�e���V���X�e�[�g�̐ݒ�
	pipelineDesc.DepthStencilState.DepthEnable = true;	//�[�x�e�X�g���s��
	pipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	//�������݋���
	pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//�p�C�v���C���Ƀ��[�g�V�O�l�`�����Z�b�g
	pipelineDesc.pRootSignature = rootSignature.Get();

	/*result = dx->GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));*/
}
#pragma endregion
#pragma region �p�C�v���C���X�e�[�g
void Cube::InitializePipelineState()
{
	HRESULT result;
	result = dx->GetDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));
}
#pragma endregion

void Cube::Update()
{
	HRESULT result;
	/*vertex->Update();
	vertBuff.Initialize(vertex, dx);*/
	pipelineDesc.pRootSignature = rootSignature.Get();
	//�p�C�v���C���X�e�[�g
	/*result = dx->GetDevice()->CreateGraphicsPipelineState(&pipe.pipelineDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));*/
	//�r���[�|�[�g�ݒ�R�}���h
	viewport.Width = window_width;
	viewport.Height = window_height;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//�r���[�|�[�g�ݒ�R�}���h���R�}���h���X�g�ɐς�
	dx->GetCommandList()->RSSetViewports(1, &viewport);

	scissorRect.left = 0;
	scissorRect.right = scissorRect.left + window_width;
	scissorRect.top = 0;
	scissorRect.bottom = scissorRect.top + window_height;
	//�V�U�[��`�ݒ�R�}���h���R�}���h���X�g�ɐς�
	dx->GetCommandList()->RSSetScissorRects(1, &scissorRect);
	//�p�C�v���C���X�e�[�g���Z�b�g
	dx->GetCommandList()->SetPipelineState(pipelineState.Get());
	//���[�g�V�O�l�`�����Z�b�g
	dx->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	//�v���~�e�B�u�`��̐ݒ�R�}���h
	dx->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	//�O�p�`���X�g
	//�萔�o�b�t�@�r���[(CBV)�̐ݒ�R�}���h
	dx->GetCommandList()->SetGraphicsRootConstantBufferView(0, constBuffMaterial->GetGPUVirtualAddress());
}
