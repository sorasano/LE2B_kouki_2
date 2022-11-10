#pragma once
#include "DirectXCommon.h"
#include "Object3D.h"
#include "d3d12.h"
#include "list"
#include "vector"
#include "d3dcompiler.h"

using namespace DirectX;
using namespace Microsoft::WRL;

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dinput8.lib")

#define PI 3.14159265359

const int fine = 64;	//���ׂ̂̍���	�ϐ��錾�p
const int fine2 = fine * fine * 2;	//�`��Ɏg�����_�̐�
const int fine3 = fine * fine * 3;	//�C���f�b�N�X�̐�
const int fine4 = fine * fine + fine;	//���_�̐�

class Sphere
{
public:
	//�V���O���g���C���X�^���X
	static Sphere* GetInstance();
	//������
	void Initialize(XMFLOAT3 size, DirectXCommon* dx_);
	void InitializeVertex(XMFLOAT3 size);	//���_������
	void InitializeIndexBuff();				//�C���f�b�N�X�o�b�t�@������	
	void InitializeVertBuff();				//���_�o�b�t�@������
	void CompileShader(const wchar_t* file, const wchar_t* file2);	//�V�F�[�_�ǂݍ���
	void InitializeRootSignature();			//���[�g�V�O�l�`��
	void InitializePipeline();				//�p�C�v���C��
	void InitializePipelineState();			//�p�C�v���C���X�e�[�g
	//�X�V
	void Update();
public:
	DirectXCommon* dx;
	ComPtr<ID3D12PipelineState> pipelineState;
	ComPtr<ID3D12Resource> constBuffMaterial;
public:
	D3D12_RECT scissorRect{};
	D3D12_VIEWPORT viewport{};

public:
	//���_�f�[�^�\����
	struct Vertex
	{
		XMFLOAT3 pos;	//���W
		XMFLOAT3 normalize;	//�@���x�N�g��
		XMFLOAT2 uv;	//uv���W
		Vertex* parent = nullptr;
	};

	struct Indices
	{
		int num;
	};
	//Vertex�֘A
	const float fineSize = fine;	//���ׂ̂̍���
	//Vertex vertices[fine2];	//�O���ɓn���p�̒��_�f�[�^
	std::vector<Vertex> vertices;	//�O���ɓn���p�̒��_�f�[�^
	std::vector<Vertex> v;	
	std::vector<Vertex> v2;	
	std::vector<Vertex> v3;
	std::vector<unsigned short> indices;
	UINT sizeVB;
	UINT sizeIB;
	float angleX, angleY;
	float oneAngle = (2 * PI) / fineSize;
	D3D12_INPUT_ELEMENT_DESC inputLayout[3];//���_���C�A�E�g	xyz���W�A�@���x�N�g���Auv���W�̏���
	//�C���f�b�N�X�o�b�t�@�֘A
	ComPtr<ID3D12Resource> indexBuff;
	D3D12_INDEX_BUFFER_VIEW ibView{};
	//���_�o�b�t�@������
	ComPtr<ID3D12Resource> vertBuff;
	D3D12_VERTEX_BUFFER_VIEW vbView{};		//���_�o�b�t�@�r���[
	//�V�F�[�_�֘A
	ID3DBlob* vsBlob;	//���_�V�F�[�_�[�I�u�W�F�N�g
	ID3DBlob* psBlob;	//�s�N�Z���V�F�[�_�[�I�u�W�F�N�g
	ID3DBlob* errorBlob;	//�G���[�I�u�W�F�N�g
	//���[�g�V�O�l�`��
	ComPtr<ID3D12RootSignature> rootSignature;
	//�p�C�v���C��
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};	//�O���t�B�b�N�X�p�C�v���C��
};

