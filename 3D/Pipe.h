#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "input.h"
#include "Masage.h"
#include "Texture.h"
#include "VertBuff.h"
#include "IndexBuff.h"
#include "Shader.h"
#include "Depth.h"
#include "RootSig.h"

#include "string"
#include "DirectXMath.h"
//#include "d3dcompiler.h"
#include "dinput.h"
#include "assert.h"
#include "DirectXTex.h"
#include "object3D.h"

using namespace DirectX;
using namespace Microsoft::WRL;

//#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dinput8.lib")

class Pipe
{
public:
	static Pipe* GetInstance();
	void Initialize(Shader shader_, RootSig rootSig_, Ver* vertex_, DirectXCommon* dx_);
	void Initialize(Shader shader_, RootSig rootSig_, Ver2* vertex_, DirectXCommon* dx_);
	void Initialize(Shader shader_, RootSig rootSig_,Ver3* vertex_, DirectXCommon* dx_);
	void Update();
public:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};	//�O���t�B�b�N�X�p�C�v���C��
	//ComPtr<ID3D12PipelineState> pipelineState;			//�p�C�v���C���X�e�[�g
	Shader shader;
	IndexBuff indexBuff;
	VertBuff vertBuff;
	Ver* vertex;
	Ver2* vertex2;
	Ver3* vertex3;
	RootSig rootSig;
	DirectXCommon* dx;
};

