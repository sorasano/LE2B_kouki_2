#pragma once
#include "Model.h"
#include "Object3D.h"
#include "Player.h"
#include "DirectXCommon.h"

class PlayerEffect
{
public:
	//シングルトンインスタンス
	PlayerEffect* GetInstance();
	PlayerEffect();
	~PlayerEffect();
	void Initialize(DirectXCommon* dx, Model* model);
	void Update(XMMATRIX& matView, XMMATRIX& matProjection);
	void Draw();
private:
	DirectXCommon* dxCommon_;
	Model* cube_;
	std::vector<Object3D> object_;

	std::vector<XMFLOAT3> translation_;
	std::vector<XMFLOAT3> scale_;
	std::vector<XMFLOAT3> rotation_;

	std::vector<XMFLOAT3> velocity_;
	std::vector<XMFLOAT3> rotVector_;
};

