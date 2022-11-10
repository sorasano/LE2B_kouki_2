#include "Model.h"
#include "Object3D.h"
#include "DirectXCommon.h"

#pragma once
class Player
{
public:
	//シングルトンインスタンス
	Player* GetInstance();
	Player();
	~Player();
	void Initialize(DirectXCommon* dx,Model* model);
	void Update(XMMATRIX& matView, XMMATRIX& matProjection);
	void Draw();
	//ゲッター　セッター　
	XMFLOAT3 GetPosition() { return position_; };
	XMFLOAT3 GetRotation() { return rotation_; };
	XMFLOAT3 GetScale() { return scale_; };
	void setPosition(XMFLOAT3 pos);
	void setRotation(XMFLOAT3 rot);
	void setScale(XMFLOAT3 sca);
private:
	DirectXCommon* dx_ = nullptr;
	Model* model_ = nullptr;
	std::unique_ptr<Object3D> object3d_;
	//アフィン変換情報
	XMFLOAT3 scale_ = { 1,1,1 };
	XMFLOAT3 rotation_ = { 0,0,0 };
	XMFLOAT3 position_ = { 0,0,0 };
	//ワールド変換行列
	XMMATRIX matWorld;
};

