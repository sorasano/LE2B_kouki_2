#include "Player.h"

Player* Player::GetInstance()
{
	static Player instance;
	return &instance;
}

Player::Player()
{
}

Player::~Player()
{
}

void Player::Initialize(DirectXCommon* dx,Model* model)
{
	//引数から受け取った値を代入
	this->dx_ = dx;
	this->model_ = model;

	//オブジェクト初期化
	Object3D* newObject = new Object3D();
	newObject->Initialize(dx_, model_);
	object3d_.reset(newObject);
}

void Player::Update(XMMATRIX& matView, XMMATRIX& matProjection)
{
	position_.x += 0.02;



	object3d_->setPosition(position_);
	object3d_->setRotation(rotation_);
	object3d_->setScale(scale_);
	//オブジェクト更新
	object3d_->Update(matView, matProjection);
}

void Player::Draw()
{
	object3d_->Draw(model_->vbView, model_->ibView);
}

void Player::setPosition(XMFLOAT3 pos)
{ 
	position_ = pos;
}

void Player::setRotation(XMFLOAT3 rot)
{
	rotation_ = rot;
}

void Player::setScale(XMFLOAT3 sca)
{
	scale_ = sca;
}
