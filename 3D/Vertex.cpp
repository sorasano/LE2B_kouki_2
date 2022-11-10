#include "Vertex.h"

Ver* Ver::GetInstance()
{
	static Ver instance;
	return &instance;
}

void Ver::Initialize(XMFLOAT3 size)
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
		for (int i = 0; i < _countof(indices) / 3; i++)
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

		sizeVB = static_cast<UINT>(sizeof(vertices[0]) * _countof(vertices));
		sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

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

Ver2* Ver2::GetInstance()
{
	static Ver2 instance;
	return &instance;
}

void Ver2::Initialize(XMFLOAT3 size)
{
	//���_�f�[�^
	Vertex v[] = {
		//�O
		{{0          , size.y / 2,          0},{},{0.5f,0.0f} },	//0
		{{ size.x / 2,-size.y / 2,-size.z / 2},{},{0.0f,1.0f} },	//2 
		{{-size.x / 2,-size.y / 2,-size.z / 2},{},{1.0f,1.0f} },	//1 
		//��
		{{			0, size.y / 2,          0},{},{0.5f,0.0f} },	//3
		{{-size.x / 2,-size.y / 2,-size.z / 2},{},{0.0f,1.0f} },	//4
		{{          0,-size.y / 2, size.z / 2},{},{1.0f,1.0f} },	//5 
		//�E
		{{0          , size.y / 2,          0},{},{0.5f,0.0f} },	//6
		{{          0,-size.y / 2, size.z / 2},{},{0.0f,1.0f} },	//7 
		{{ size.x / 2,-size.y / 2,-size.z / 2},{},{1.0f,1.0f} },	//8 
		//��
		{{-size.x / 2,-size.y / 2,-size.z / 2},{},{0.5f,0.0f} },	//9
		{{ size.x / 2,-size.y / 2,-size.z / 2},{},{0.0f,1.0f} },	//10 
		{{			0,-size.y / 2, size.z / 2},{},{1.0f,1.0f} },	//11 
	};
	//�C���f�b�N�X�f�[�^
	unsigned short in[] =
	{
		//�O
		0,1,2,	//�O�p�`1��
		3,4,5,	//�O�p�`2��
		6,7,8,
		9,10,11
	};

	//���_���W�Auv���W�A�C���f�b�N�X�f�[�^����
	for (int i = 0; i < 12; i++)
	{
		vertices[i] = v[i];
	}

	for (int i = 0; i < 12; i++)
	{
		indices[i] = in[i];
	}

	//�@���̌v�Z
	for (int i = 0; i < 12 / 3; i++)
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

	sizeVB = static_cast<UINT>(sizeof(vertices[0]) * _countof(vertices));
	sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

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

Ver3* Ver3::GetInstance()
{
	static Ver3 instance;
	return &instance;
}

void Ver3::Initialize(XMFLOAT3 size)
{
	angleY = 0;
	//���_�f�[�^
	float x, y, z;
	for (int i = 0; i < f2; i++)
	{
		if (i == 0 || i % 4 == 0)
		{
			if (i == 0)
			{
				angleX = 0;
			}
			if (i == 0 || i % (f * 4) == 0)
			{
				angleY = (2 * PI) * ((float)(i + f * 4) / (float)(f * f * 4));
			}
			else
			{
				angleY += oneAngle;
			}

			v[i].pos.x = size.x * cos(angleX) * sin(angleY);
			v[i].pos.y = size.y * cos(angleY);
			v[i].pos.z = size.z * sin(angleX) * sin(angleY);
			
		}

		if (i == 1 || i % 4 == 1)
		{
			angleY -= oneAngle;

			v[i].pos.x = size.x * cos(angleX) * sin(angleY);
			v[i].pos.y = size.y * cos(angleY);
			v[i].pos.z = size.z * sin(angleX) * sin(angleY);
			
		}
		if (i == 2 || i % 4 == 2)
		{
			angleX += oneAngle;
			angleY += oneAngle;

			v[i].pos.x = size.x * cos(angleX) * sin(angleY);
			v[i].pos.y = size.y * cos(angleY);
			v[i].pos.z = size.z * sin(angleX) * sin(angleY);
			
		}
		if (i == 3 || i % 4 == 3)
		{
			angleY -= oneAngle;

			v[i].pos.x = size.x * cos(angleX) * sin(angleY);
			v[i].pos.y = size.y * cos(angleY);
			v[i].pos.z = size.z * sin(angleX) * sin(angleY);
			
		}
	}

	unsigned short in[f3];
	for (int i = 0; i < f3; i++)
	{
		double num_ = ((i / 6) * 6) * 2 / 3;	
		if (i == 0 || i % 6 == 0)							{ in[i] = num_; }
		if (i == 1 || i == 4 || i % 6 == 1 || i % 6 == 4)	{ in[i] = num_ + 1; }
		if (i == 2 || i == 3 || i % 6 == 2 || i % 6 == 3)	{ in[i] = num_ + 2; }
		if (i == 5 || i % 6 == 5)							{ in[i] = num_ + 3; }
	}

	angleY = 0;
	angleX = 0;
	//���_�f�[�^	�ォ�珇�ԂɊ��蓖�Ă�
	for (int i = 0; i < f4 ; i++)
	{
		if (i == 0 || i % f == 0)
		{
			angleX = 0;
		}
		else
		{
			angleX += oneAngle;
		}
		if (i == 0)
		{
			angleY = 0;
		}
		else if (i != 0 && i >= f && i % f == 0)
		{
			angleY = (2 * PI) * ((float)(i) / (float)(f * f));
		}
		v2[i].pos.x = size.x * cos(angleX) * sin(angleY);
		v2[i].pos.y = size.y * cos(angleY);
		v2[i].pos.z = size.z * sin(angleX) * sin(angleY);
		v3[i].pos.x = v2[i].pos.x;
		v3[i].pos.y = v2[i].pos.y;
		v3[i].pos.z = v2[i].pos.z;
	}


	for (int i = 0; i < f2; i++)
	{
		for (int j = 0; j < f4; j++)
		{
			//uv(0.0f,0.0f)
			if (i == 1 || i % 4 == 1)
			{
				if (i == 1)
				{
					v[i].parent = &v2[0];
				}
				else if (i % 4 == 1 && i != 1)
				{
					v[i].parent = &v2[i / 4];
				}
			}
			//uv(1.0f,0.0f)
			if (i == 3 || i % 4 == 3)
			{
				if (i == 3)
				{
					v[i].parent = &v2[1];
				}
				else if (i % 4 == 3)
				{
					if (i % (f * 4) != (f * 4) - 1 && i != (f * 4) - 1)
					{
						v[i].parent = &v2[(i + 1) / 4];
					}
					if (i % (f * 4) == (f * 4) - 1 || i == (f * 4) - 1)
					{
						v[i].parent = &v2[(i / 4) - (f - 1)];
					}
				}
			}

			//uv(0.0f,1.0f)
			if (i == 0 || i % 4 == 0)
			{
				if (i == 0)
				{
					v[i].parent = &v2[f];
				}
				else if (i % 4 == 0 && i != 0)
				{
					v[i].parent = &v2[(i / 4) + f];
				}
			}

			if (i == 2 || i % 4 == 2)
			{
				if (i == 2)
				{
					v[i].parent = &v2[f + 1];
				}
				else if (i % 4 == 2)
				{
					if (i % (f * 4) != (f * 4) - 2 && i != (f * 4) - 2)
					{
						v[i].parent = &v2[(i + 2) / 4 + f];
					}
					if (i % (f * 4) == (f * 4) - 2 || i == (f * 4) - 2)
					{
						v[i].parent = &v2[(i / 4) - (f - 1) + f];
					}
				}
			}
		}
	}


	//���_���W�Auv���W�A�C���f�b�N�X�f�[�^����
	for (int i = 0; i < f2; i++)
	{
		vertices[i] = v[i];
	}

	for (int i = 0; i < f3; i++)
	{
		indices[i] = in[i];
	}

	//�@���̌v�Z
	for (int i = 0; i < f3 / 3; i++)
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

	sizeVB = static_cast<UINT>(sizeof(vertices[0]) * _countof(vertices));
	sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

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

void Ver3::Initialize2(XMFLOAT3 size)
{
	/*float x, y, z;*/
	//���_�f�[�^	�ォ�珇�ԂɊ��蓖�Ă�
	for (int i = 0; i < f4; i++)
	{
		v2[i].pos.x = v3[i].pos.x;
		v2[i].pos.y = v3[i].pos.y;
		v2[i].pos.z = v3[i].pos.z;
	}
}

void Ver3::Update()
{
	//�e���_�����Ƃɒ��_���ړ�
	for (int i = 0; i < f2; i++)
	{
		if (v[i].parent != nullptr)
		{
			v[i].pos.x = v[i].parent->pos.x;
			v[i].pos.y = v[i].parent->pos.y;
			v[i].pos.z = v[i].parent->pos.z;
		}
	}
	 //���_���W���X�V�Auv���W�A�C���f�b�N�X�f�[�^���X�V
	for (int i = 0; i < fineSize * fineSize * 2; i++)
	{
		vertices[i] = v[i];
	}

	//�@���̌v�Z
	for (int i = 0; i < fineSize * fineSize * 3 / 3; i++)
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

	sizeVB = static_cast<UINT>(sizeof(vertices[0]) * _countof(vertices));
	sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

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
