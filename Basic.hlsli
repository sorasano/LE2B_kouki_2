//�}�e���A��
cbuffer ConstBufferDataMaterial : register(b1)
{
	matrix mat;
}

//���_�V�F�[�_�[�̏o�͍\����
//�i���_�V�F�[�_�[����s�N�Z���V�F�[�_�[�ւ̂����Ɏg�p����j
struct VSOutput
{
	//�V�X�e���p���_���W
	float4 svpos : SV_POSITION;
	//�@���x�N�g��
	float3 normal : NORMAL;
	// uv�l
	float2 uv : TEEXCOORD;
};