#include "Depth.h"

Depth* Depth::GetInstance()
{
	static Depth instance;
	return &instance;
}

void Depth::Initialize(DirectXCommon* dx_)
{
	HRESULT result;
	dx = dx_;

	//���\�[�X�ݒ�
	D3D12_RESOURCE_DESC depthResorceDesc{};
	depthResorceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResorceDesc.Width = window_width;	//�����_�[�^�[�Q�b�g�ɍ��킹��
	depthResorceDesc.Height = window_height;	//�����_�[�^�[�Q�b�g�ɍ��킹��
	depthResorceDesc.DepthOrArraySize = 1;
	depthResorceDesc.Format = DXGI_FORMAT_D32_FLOAT;	//�[�x�l�t�H�[�}�b�g
	depthResorceDesc.SampleDesc.Count = 1;
	depthResorceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//�f�v�X�X�e���V��

	//�k�x�l�p�q�[�v�v���p�e�B
	D3D12_HEAP_PROPERTIES depthHeapProp{};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	//�[�x�l�̃N���A�ݒ�
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;	//�[�x�l1.0f(�ő�l)�ŃN���A
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;	//�[�x�l�t�H�[�}�b�g

	//���\�[�X����
	ComPtr<ID3D12Resource> depthBuff;
	result = dx->GetDevice()->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResorceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,	//�[�x�l�������݂Ɏg�p
		&depthClearValue,
		IID_PPV_ARGS(&depthBuff)
	);

	//�[�x�r���[�p�f�X�N���v�^�q�[�v�쐬
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;	//�[�x�r���[��1��
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//�f�v�X�X�e���V���r���[
	result = dx->GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));

	//�[�x�X�e���V���r���[�̐���
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;	//�[�x�l�t�H�[�}�b�g
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dx->GetDevice()->CreateDepthStencilView(
		depthBuff.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);
}

void Depth::Update(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
	dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	dx->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
}

void Depth::Update2()
{
	dx->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}
