#include "Sprite.h"

PipelineSet Sprite::SpriteCreateGraphicsPipeline(ID3D12Device* dev) {

	HRESULT result;
	ComPtr<ID3DBlob> vsBlob; // ���_�V�F�[�_�I�u�W�F�N�g
	ComPtr<ID3DBlob> psBlob; // �s�N�Z���V�F�[�_�I�u�W�F�N�g
	ComPtr<ID3DBlob> errorBlob; // �G���[�I�u�W�F�N�g

	// ���_�V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		L"SpriteVS.hlsl", // �V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"main", "vs_5_0", // �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0,
		&vsBlob, &errorBlob);

	// �G���[�Ȃ�
	if (FAILED(result)) {
		// errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	// �s�N�Z���V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		L"SpritePS.hlsl",   // �V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"main", "ps_5_0", // �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0,
		&psBlob, &errorBlob);

	// �G���[�Ȃ�
	if (FAILED(result)) {
		// errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	// ���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ // xyz���W(1�s�ŏ������ق������₷��)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},

		{ // uv���W(1�s�ŏ������ق������₷��)
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};




	// �f�X�N���v�^�����W�̐ݒ�
	CD3DX12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	//descriptorRange.NumDescriptors = 1;         //��x�̕`��Ɏg���e�N�X�`����1���Ȃ̂�1
	//descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	//descriptorRange.BaseShaderRegister = 0;     //�e�N�X�`�����W�X�^0��
	//descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//// ���[�g�p�����[�^�̐ݒ�
	//D3D12_ROOT_PARAMETER rootParams[3] = {};
	//// �萔�o�b�t�@0��
	//rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   // ���
	//rootParams[0].Descriptor.ShaderRegister = 0;                   // �萔�o�b�t�@�ԍ�
	//rootParams[0].Descriptor.RegisterSpace = 0;                    // �f�t�H���g�l
	//rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;  // �S�ẴV�F�[�_���猩����
	//// �e�N�X�`�����W�X�^0��
	//rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;   //���
	//rootParams[1].DescriptorTable.pDescriptorRanges = &descriptorRange;		  //�f�X�N���v�^�����W
	//rootParams[1].DescriptorTable.NumDescriptorRanges = 1;              		  //�f�X�N���v�^�����W��
	//rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;               //�S�ẴV�F�[�_���猩����
	//// �萔�o�b�t�@1��
	//rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   //���
	//rootParams[2].Descriptor.ShaderRegister = 1;		  //�f�X�N���v�^�����W
	//rootParams[2].Descriptor.RegisterSpace = 0;              		  //�f�X�N���v�^�����W��
	//rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;                // �f�t�H���g�l

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

	// �O���t�B�b�N�X�p�C�v���C���ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

	// �V�F�[�_�[�̐ݒ�
	pipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	// �T���v���}�X�N�̐ݒ�
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // �W���ݒ�

	// ���X�^���C�U�̐ݒ�
	pipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // �J�����O���Ȃ�
	//pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // �|���S�����h��Ԃ�
	//pipelineDesc.RasterizerState.DepthClipEnable = true; // �[�x�N���b�s���O��L����

	// �����_�[�^�[�Q�b�g�̃u�����h�ݒ�
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = pipelineDesc.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // RBGA�S�Ẵ`�����l����`��

	blenddesc.BlendEnable = true;                   // �u�����h��L���ɂ���
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;    // ���Z
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;      // �\�[�X�̒l��100% �g��
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;    // �f�X�g�̒l��  0% �g��

	// ����������
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;             // ���Z
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;         // �\�[�X�̃A���t�@�l
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;    // 1.0f-�\�[�X�̃A���t�@�l

	// ���_���C�A�E�g�̐ݒ�
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);

	// �}�`�̌`��ݒ�
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// ���̑��̐ݒ�
	pipelineDesc.NumRenderTargets = 1; // �`��Ώۂ�1��
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0�`255�w���RGBA
	pipelineDesc.SampleDesc.Count = 1; // 1�s�N�Z���ɂ�1��T���v�����O

	//�f�v�X�X�e���V���X�e�[�g�̐ݒ�
	pipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; // ��ɏ㏑�����[��

	pipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pipelineDesc.DepthStencilState.DepthEnable = false;
	//pipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	//pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// ���[�g�V�O�l�`��
	//ComPtr<ID3D12RootSignature> rootSignature;
	// ���[�g�V�O�l�`���̐ݒ�
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParams; //���[�g�p�����[�^�̐擪�A�h���X
	rootSignatureDesc.NumParameters = _countof(rootParams);        //���[�g�p�����[�^��

	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	//�p�C�v���C���Ɓ�V�O�l�`���̃Z�b�g
	PipelineSet pipelineSet;

	// ���[�g�V�O�l�`���̃V���A���C�Y
	ComPtr<ID3DBlob> rootSigBlob;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob, &errorBlob);
	assert(SUCCEEDED(result));
	result = dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&pipelineSet.rootsignature));
	assert(SUCCEEDED(result));

	// �p�C�v���C���Ƀ��[�g�V�O�l�`�����Z�b�g
	pipelineDesc.pRootSignature = pipelineSet.rootsignature.Get();

	// �p�C�v�����X�e�[�g�̐���
	result = dev->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineSet.pipelinestate));
	assert(SUCCEEDED(result));

	// �f�X�N���v�^�q�[�v�𐶐�	
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
	descHeapDesc.NumDescriptors = 1; // �V�F�[�_�[���\�[�X�r���[1��
	result = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap));//����
	if (FAILED(result)) {
		assert(0);
	}

	//�p�C�v���C���ƃ��[�g�V�O�l�`����Ԃ�
	return pipelineSet;
}

void Sprite::SetPiplineSet(PipelineSet piplineSet)
{
	//Sprite::PipelineSet::pipelinestate = piplineSet.pipelinestate;
}

Sprite Sprite::SpriteCreate(ID3D12Device* dev, int window_width, int window_height) {

	HRESULT result = S_FALSE;

	//�V�����X�v���C�g�����
	Sprite sprite{};

	//���_�f�[�^
	VertexPosUv vertices[] = {
		{{	0.0f, 100.0f,	0.0f},{0.0f,1.0f}},
		{{	0.0f,	0.0f,	0.0f},{0.0f,0.0f}},
		{{100.0f, 100.0f,	0.0f},{1.0f,1.0f}},
		{{100.0f,	0.0f,	0.0f},{1.0f,0.0f}},
	};

	// �q�[�v�v���p�e�B
	CD3DX12_HEAP_PROPERTIES heapPropsVertexBuffer = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// ���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC resourceDescVertexBuffer =
		CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));

	//���_�o�b�t�@����
	result = dev->CreateCommittedResource(
		&heapPropsVertexBuffer, // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resourceDescVertexBuffer, // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&sprite.vertBuff));
	assert(SUCCEEDED(result));

	//���_�o�b�t�@�ւ̃f�[�^�]��
	VertexPosUv* vertMap = nullptr;
	result = sprite.vertBuff->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	sprite.vertBuff->Unmap(0, nullptr);

	// ���_�o�b�t�@�r���[�̍쐬
	// GPU���z�A�h���X
	sprite.vbView.BufferLocation = sprite.vertBuff->GetGPUVirtualAddress();
	// ���_�o�b�t�@�̃T�C�Y
	sprite.vbView.SizeInBytes = sizeof(vertices);
	// ���_1���̃f�[�^�T�C�Y
	sprite.vbView.StrideInBytes = sizeof(vertices[0]);

	// �q�[�v�v���p�e�B
	CD3DX12_HEAP_PROPERTIES heapPropsConstantBuffer = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// ���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC resourceDescConstantBuffer =
		CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff);

	// �萔�o�b�t�@�̐���
	result = dev->CreateCommittedResource(
		&heapPropsConstantBuffer, // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resourceDescConstantBuffer, // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&sprite.constBuff));
	assert(SUCCEEDED(result));

	// �萔�o�b�t�@�Ƀf�[�^�]��
	ConstBufferData* constMap = nullptr;
	result = sprite.constBuff->Map(0, nullptr, (void**)&constMap); // �}�b�s���O
	constMap->color = XMFLOAT4(1, 1, 1, 1);
	assert(SUCCEEDED(result));

	//���s���e�s��
	constMap->mat = XMMatrixOrthographicOffCenterLH(0.0f, window_width, window_height, 0.0f, 0.0f, 1.0f);
	sprite.constBuff->Unmap(0, nullptr);

	return sprite;
}
//�X�v���C�g���ʃO���t�B�b�N�X�R�}���h�̃Z�b�g
void Sprite::SpriteCommonBeginDraw(ID3D12GraphicsCommandList* cmdList,const SpriteCommon& spriteCommon) {

	// �p�C�v���C���X�e�[�g�ƃ��[�g�V�O�l�`���̐ݒ�R�}���h
	cmdList->SetPipelineState(spriteCommon.pipelineSet.pipelinestate.Get());
	cmdList->SetGraphicsRootSignature(spriteCommon.pipelineSet.rootsignature.Get());

	// �v���~�e�B�u�`��̐ݒ�R�}���h
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); // �O�p�`���X�g

	//�e�N�X�`���p�ŃX�N���v�^�q�[�v�̐ݒ�
	ID3D12DescriptorHeap* ppHeaps[] = { spriteCommon.descHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

//�X�v���C�g�P�̕`��

void Sprite::SpriteDraw(ID3D12GraphicsCommandList* cmdList,const Sprite& sprite,const SpriteCommon& spriteCommon, ID3D12Device* dev) {

	//this->cmmandList = dx->GetCommandList();

	// ���_�o�b�t�@�̐ݒ�R�}���h
	cmdList->IASetVertexBuffers(0, 1, &sprite.vbView);

	// �萔�o�b�t�@(CBV)�̐ݒ�R�}���h
	cmdList->SetGraphicsRootConstantBufferView(0, sprite.constBuff->GetGPUVirtualAddress());

	//�V�F�[�_�[���\�[�X�r���[���Z�b�g
	cmdList->SetGraphicsRootDescriptorTable(1, CD3DX12_GPU_DESCRIPTOR_HANDLE(spriteCommon.descHeap->GetGPUDescriptorHandleForHeapStart(),
		sprite.texNumber, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)));

	//�|���S���̕`��
	cmdList->DrawInstanced(4, 1, 0, 0);

}

SpriteCommon Sprite::SpriteCommonCreate(ID3D12Device* dev, int window_width, int window_height) {
	HRESULT result = S_FALSE;

	//�V���ȃX�v���C�g���ʃf�[�^�𐶐�
	SpriteCommon spriteCommon{};

	//�X�v���C�g�p�p�C�v���C������
	spriteCommon.pipelineSet = SpriteCreateGraphicsPipeline(dev);

	//���s���e�s�񐶐�
	spriteCommon.matProjrction = XMMatrixOrthographicOffCenterLH(0.0f, (float)window_width, (float)window_height, 0.0f, 0.0f, 1.0f);

	//HRESULT result = S_FALSE;

	//�f�X�N���v�^�q�[�v�𐶐�
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = SpriteCommon::spriteSRVCount;
	result = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&spriteCommon.descHeap));

	//���������X�v���C�g�����Ă����Ł[����Ԃ�
	return spriteCommon;
}

void Sprite::SpriteUpdate(Sprite& sprite, const SpriteCommon& spriteCommon) {

	time++;

	sprite.matWorld = XMMatrixIdentity();

	sprite.matWorld *= XMMatrixRotationZ(XMConvertToRadians(sprite.rotation));

	sprite.matWorld *= XMMatrixTranslation(sprite.position.x, sprite.position.y, sprite.position.z);
	

	// �萔�o�b�t�@�Ƀf�[�^�]��
	ConstBufferData* constMap = nullptr;
	HRESULT result = sprite.constBuff->Map(0, nullptr, (void**)&constMap); // �}�b�s���O
	constMap->mat = sprite.matWorld * spriteCommon.matProjrction;
	constMap->time = time;
	sprite.constBuff->Unmap(0, nullptr);
}

void Sprite::SpriteCommonLoadTexture(SpriteCommon& spriteCommon, UINT texnumber, const wchar_t* filename, ID3D12Device* dev) {

	assert(texnumber <= SpriteCommon::spriteSRVCount - 1);

	HRESULT result;
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	// WIC�e�N�X�`���̃��[�h
	result = LoadFromWICFile(filename, WIC_FLAGS_NONE, &metadata, scratchImg);
	assert(SUCCEEDED(result));

	ScratchImage mipChain{};
	// �~�b�v�}�b�v����
	result = GenerateMipMaps(
		scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain);
	if (SUCCEEDED(result)) {
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}

	// �ǂݍ��񂾃f�B�t���[�Y�e�N�X�`����SRGB�Ƃ��Ĉ���
	metadata.format = MakeSRGB(metadata.format);

	// ���\�[�X�ݒ�
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format, metadata.width, (UINT)metadata.height, (UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels);

	// �q�[�v�v���p�e�B
	CD3DX12_HEAP_PROPERTIES heapProps =
		CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);

	// �e�N�X�`���p�o�b�t�@�̐���
	result = dev->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // �e�N�X�`���p�w��
		nullptr, IID_PPV_ARGS(&spriteCommon.texBuff[texnumber]));
	assert(SUCCEEDED(result));

	// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	for (size_t i = 0; i < metadata.mipLevels; i++) {
		const Image* img = scratchImg.GetImage(i, 0, 0); // ���f�[�^���o
		result = spriteCommon.texBuff[texnumber]->WriteToSubresource(
			(UINT)i,
			nullptr,              // �S�̈�փR�s�[
			img->pixels,          // ���f�[�^�A�h���X
			(UINT)img->rowPitch,  // 1���C���T�C�Y
			(UINT)img->slicePitch // 1���T�C�Y
		);
		assert(SUCCEEDED(result));
	}

	// �V�F�[�_���\�[�X�r���[�쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // �ݒ�\����
	D3D12_RESOURCE_DESC resDesc = spriteCommon.texBuff[texnumber]->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1;

	dev->CreateShaderResourceView(spriteCommon.texBuff[texnumber].Get(), //�r���[�Ɗ֘A�t����o�b�t�@
		&srvDesc, //�e�N�X�`���ݒ���
		CD3DX12_CPU_DESCRIPTOR_HANDLE(spriteCommon.descHeap->GetCPUDescriptorHandleForHeapStart(), texnumber, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
	);

	//return S_OK;
}

void Sprite::SpriteTransferVertexBuffer(const Sprite& sprite) {

	HRESULT result = S_FALSE;

	//���_�f�[�^
	VertexPosUv vertices[] = {
		{{},{0.0f,1.0f}},
		{{},{0.0f,0.0f}},
		{{},{1.0f,1.0f}},
		{{},{1.0f,0.0f}},
	};

	enum { LB, LT, RB, RT };

	vertices[LB].pos = { 0.0f,sprite.size.y,0.0f };
	vertices[LT].pos = { 0.0f,0.0f,0.0f };
	vertices[RB].pos = { sprite.size.x,sprite.size.y,0.0f };
	vertices[RT].pos = { sprite.size.x,0.0f,0.0f };

	VertexPosUv* vertMap = nullptr;
	result = sprite.vertBuff->Map(0, nullptr, (void**)&vertMap);
	memcpy(vertMap, vertices, sizeof(vertices));
	sprite.vertBuff->Unmap(0, nullptr);
}

void Sprite::Release()
{
	delete dx;
}

