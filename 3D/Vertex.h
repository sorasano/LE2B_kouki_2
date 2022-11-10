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
#include "math.h"
#include "Gravity.h"

#define PI 3.141592653589793238462643

using namespace DirectX;
using namespace Microsoft::WRL;

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

class Ver
{
public:
	static Ver* GetInstance();
	void Initialize(XMFLOAT3 size);
public:
	Vertex vertices[24];
	unsigned short indices[36];
	UINT sizeVB;
	UINT sizeIB;
	D3D12_INPUT_ELEMENT_DESC inputLayout[3];//���_���C�A�E�g	xyz���W�A�@���x�N�g���Auv���W�̏���
};

class Ver2
{
public:
	static Ver2* GetInstance();
	void Initialize(XMFLOAT3 size);
public:
	Vertex vertices[12];
	unsigned short indices[12];
	UINT sizeVB;
	UINT sizeIB;
	D3D12_INPUT_ELEMENT_DESC inputLayout[3];//���_���C�A�E�g	xyz���W�A�@���x�N�g���Auv���W�̏���
};

const int f = 32;	//���ׂ̂̍���	�ϐ��錾�p
const int f2 = f * f * 2;	//�`��Ɏg�����_�̐�
const int f3 = f * f * 3;	//�C���f�b�N�X�̐�
const int f4 = f * f + f;	//���_�̐�

class Ver3
{
public:
	static Ver3* GetInstance();
	void Initialize(XMFLOAT3 size);
	void Initialize2(XMFLOAT3 size);
	void Update();
public:
	const float fineSize = f;	//���ׂ̂̍���
	Vertex vertices[f2];	//�O���ɓn���p�̒��_�f�[�^
	Vertex v[f2], v2[f4],v3[f4];	//�v�Z�p���_�f�[�^
	unsigned short indices[f3];
	UINT sizeVB;
	UINT sizeIB;
	float angleX, angleY;
	float oneAngle = (2 * PI) / fineSize;
	D3D12_INPUT_ELEMENT_DESC inputLayout[3];//���_���C�A�E�g	xyz���W�A�@���x�N�g���Auv���W�̏���
};