#pragma once
#include <Windows.h>
#include <D3dx12.h>

#include <DirectXMath.h>
using namespace DirectX;

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <wrl.h>

using namespace Microsoft::WRL;

#include <cassert>

#include <DirectXTex.h>

#include "DirectXCommon.h"

#include "Sprite.h"

class PostEffect
{
	//-----------�X�v���C�g----------

public:


	void SetPiplineSet(PipelineSet piplineSet);

	PipelineSet SpriteCreateGraphicsPipeline(ID3D12Device* dev);

	//�X�v���C�g1�����̃f�[�^

private:
	//���_�o�b�t�@
	ComPtr<ID3D12Resource> vertBuff;
	//���_�o�b�t�@�r���[
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//�萔�o�b�t�@
	ComPtr<ID3D12Resource> constBuff;
	//Z�����̉�]�p
	float rotation = 0.0f;
	//���W
	XMFLOAT3 position = { 0,0,0 };
	//���[���h�s��
	XMMATRIX matWorld;

	UINT texNumber = 0;

	XMFLOAT2 size = { 100,100 };

	float time;

	struct ConstBufferData {
		XMFLOAT4 color; // �F (RGBA)
		XMMATRIX mat; //���W
		float time;
	};

	//�R�}���h���X�g
	ComPtr<ID3D12GraphicsCommandList> cmdList;
	DirectXCommon* dx = nullptr;
	ComPtr<ID3D12DescriptorHeap> descHeap;
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};

public:

	//�X�v���C�g����
	PostEffect SpriteCreate(ID3D12Device* dev, int window_width, int window_height);

	//�X�v���C�g���ʃf�[�^����
	PostEffect SpriteCommonCreate(ID3D12Device* dev, int window_width, int window_height);

	//�X�v���C�g���ʃO���t�B�b�N�X�R�}���h�̃Z�b�g
	void SpriteCommonBeginDraw(ID3D12GraphicsCommandList* cmdList, const SpriteCommon& spriteCommon);

	//�X�v���C�g�P�̕`��

	void SpriteDraw(ID3D12GraphicsCommandList* cmdList, const PostEffect& sprite, const SpriteCommon& spriteCommon, ID3D12Device* dev);

	//�X�v���C�g�P�̍X�V
	void SpriteUpdate(PostEffect& sprite, const SpriteCommon& spriteCommon);

	//�X�v���C�g���ʃe�N�X�`���ǂݍ���
	void SpriteCommonLoadTexture(SpriteCommon& spriteCommon, UINT texnumber, const wchar_t* filename, ID3D12Device* dev);

	//�X�v���C�g�P�̒��_�o�b�t�@�̓]��
	void SpriteTransferVertexBuffer(const PostEffect& sprite);

	//�Z�b�^�[
	void SetTexNumber(UINT texnumber) { this->texNumber = texnumber; }

	void SetPosition(XMFLOAT3 position) { this->position = position; }
	void SetScale(XMFLOAT2 scale) { this->size = scale; }


	void Release();
};

