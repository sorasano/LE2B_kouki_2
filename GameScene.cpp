
#include "GameScene.h"

GameScene::GameScene()
{
}

GameScene::~GameScene()
{
}

void GameScene::Initialize(DirectXCommon* dxCommon, Input* input)
{
	this->dxCommon_ = dxCommon;
	this->input_ = input;

	//プレイヤーのモデル初期化
	Model* newModel = new Model();
	newModel->Initialize(dxCommon_, "fighter", "Resources/fighter.png");
	playerModel_.reset(newModel);

	//プレイヤー初期化
	Player* newPlayer = new Player();
	newPlayer->Initialize(dxCommon, playerModel_.get());
	player_.reset(newPlayer);

	//カメラ初期化
	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));


	//----スプライト----
	
	//スプライト共通データ生成
	spriteCommon_ = sprite_->SpriteCommonCreate(dxCommon_->GetDevice(), 1280, 720);

	//スプライトテクスチャ読み込み
	sprite_->SpriteCommonLoadTexture(spriteCommon_, 0, L"Resources/texture.jpg", dxCommon_->GetDevice());
	sprite_->SpriteCommonLoadTexture(spriteCommon_, 1, L"Resources/texture2.jpg", dxCommon_->GetDevice());
	sprite_->SpriteCommonLoadTexture(spriteCommon_, 2, L"Resources/texture3.jpg", dxCommon_->GetDevice());
	sprite_->SpriteCommonLoadTexture(spriteCommon_, 3, L"Resources/testTexture.png", dxCommon_->GetDevice());

	//スプライトの生成
	titleSprite_ = titleSprite_.SpriteCreate(dxCommon_->GetDevice(), 1280, 720);

	//テクスチャ番号セット
	titleSprite_.SetTexNumber(3);
	//テクスチャサイズ設定
	titleSprite_.SetScale(XMFLOAT2(1280, 720));
	//反映
	titleSprite_.SpriteTransferVertexBuffer(titleSprite_);

	//スプライト用パイプライン生成呼び出し
	PipelineSet spritePipelineSet = sprite_->SpriteCreateGraphicsPipeline(dxCommon_->GetDevice());

}

void GameScene::Update()
{
	player_->Update(matView, matProjection);

	//画像変更
	titleSprite_.SetTexNumber(0);

	//スケール変更と更新
	titleSprite_.SetScale(XMFLOAT2(100, 100));
	titleSprite_.SpriteTransferVertexBuffer(titleSprite_);

	//ポジション変更と更新
	titleSprite_.SetPosition(XMFLOAT3(100, 100, 0));
	titleSprite_.SpriteUpdate(titleSprite_,spriteCommon_);
}

void GameScene::Draw()
{
	player_->Draw();

	//スプライト共通コマンド
	sprite_->SpriteCommonBeginDraw(dxCommon_->GetCommandList(), spriteCommon_);

	titleSprite_.SpriteDraw(dxCommon_->GetCommandList(),titleSprite_,spriteCommon_, dxCommon_->GetDevice());

}
