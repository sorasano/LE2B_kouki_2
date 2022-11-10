#include "Square2.h"

Square2* Square2::GetInstance()
{
	static Square2 instance;
	return &instance;
}

void Square2::Initialize(XMFLOAT3 size, DirectXCommon* dx_)
{
	dx = dx_;
	HRESULT result;

	vertex = Ver::GetInstance();
	vertex->Initialize(size);
	indexBuff.GetInstance();
	indexBuff.Initialize(vertex, dx);
	vertBuff.GetInstance();
	vertBuff.Initialize(vertex, dx);
	shader.GetInstance();
	shader.compileVs(L"BasicVS.hlsl");
	shader.compilePs(L"BasicPS.hlsl");
	rootSig.GetInstance();
	rootSig.Initialize(shader, dx);
	pipe.GetInstance();
	pipe.Initialize(shader, rootSig, vertex, dx);

	//�p�C�v���C���X�e�[�g
	result = dx->GetDevice()->CreateGraphicsPipelineState(&pipe.pipelineDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));


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

void Square2::Update()
{
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
	dx->GetCommandList()->SetGraphicsRootSignature(rootSig.rootSignature.Get());
	//�v���~�e�B�u�`��̐ݒ�R�}���h
	dx->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	//�O�p�`���X�g
	//�萔�o�b�t�@�r���[(CBV)�̐ݒ�R�}���h
	dx->GetCommandList()->SetGraphicsRootConstantBufferView(0, constBuffMaterial->GetGPUVirtualAddress());
}
