#pragma once
#include "Windows.h"
#include "d3d12.h"
#include "dxgi1_6.h"
#include "cassert"
#include "vector"
#include "string"
#include "DirectXMath.h"
#include "assert.h"
#include "DirectXTex.h"
#include "wrl.h"

#include "DirectXCommon.h"

using namespace DirectX;
using namespace Microsoft::WRL;

////�摜�f�[�^�\����
//struct texData
//{
//	//�摜
//	TexMetadata metadata;
//	ScratchImage scratchImg;
//	//�~�b�v�}�b�v
//	ScratchImage mipChain;
//	//�e�N�X�`���o�b�t�@
//	ComPtr<ID3D12Resource> texBuff;
//	//�e�N�X�`���[��GPU�̃n���h��
//	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle;
//	//�摜�p�f�X�N���v�^�q�[�v
//	ComPtr<ID3D12DescriptorHeap> srvHeap;
//};

class Texture
{
public:
	static Texture* GetInstance();
	void Initialize(const wchar_t* szFile, DirectXCommon* dx, int texNum);
	void Initialize(DirectXCommon *dx,int texNum);
	void Draw();
	void SetImageData(XMFLOAT4 color);

	//�Q�b�^�[
	ID3D12Resource* GetTexBuff() { return texBuff.Get(); }
	ID3D12DescriptorHeap* GetSrvHeap() { return srvHeap.Get(); }
private:
	//�摜
	TexMetadata metadata;
	ScratchImage scratchImg;
	//�~�b�v�}�b�v
	ScratchImage mipChain;
	//�e�N�X�`���o�b�t�@
	ComPtr<ID3D12Resource> texBuff;
	//�e�N�X�`���[��GPU�̃n���h��
	D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle;
	//�摜�p�f�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> srvHeap;
	const size_t textureWidth = 256;
	const size_t textureHeight = 256;
	const size_t imageDataCount = textureWidth * textureHeight;
	XMFLOAT4* imageData = new XMFLOAT4[imageDataCount];
public:
	DirectXCommon* dx_ ;
};

