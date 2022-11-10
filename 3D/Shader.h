#pragma once
#include "d3dcompiler.h"
#include "assert.h"
#include "DirectXTex.h"
#include "object3D.h"

using namespace DirectX;
using namespace Microsoft::WRL;

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dinput8.lib")


class Shader
{
public:
	static Shader* GetInstance();
	void compileVs(const wchar_t* file);
	void compilePs(const wchar_t* file);
public:
	ID3DBlob *vsBlob;	//���_�V�F�[�_�[�I�u�W�F�N�g
	ID3DBlob *psBlob;	//�s�N�Z���V�F�[�_�[�I�u�W�F�N�g
	ID3DBlob *errorBlob;	//�G���[�I�u�W�F�N�g
};