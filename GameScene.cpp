
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

	//�v���C���[�̃��f��������
	Model* newModel = new Model();
	newModel->Initialize(dxCommon_, "fighter", "Resources/fighter.png");
	playerModel_.reset(newModel);

	//�v���C���[������
	Player* newPlayer = new Player();
	newPlayer->Initialize(dxCommon, playerModel_.get());
	player_.reset(newPlayer);

	//�J����������
	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));


	//----�X�v���C�g----
	
	//�X�v���C�g���ʃf�[�^����
	spriteCommon_ = sprite_->SpriteCommonCreate(dxCommon_->GetDevice(), 1280, 720);

	//�X�v���C�g�e�N�X�`���ǂݍ���
	sprite_->SpriteCommonLoadTexture(spriteCommon_, 0, L"Resources/texture.jpg", dxCommon_->GetDevice());
	sprite_->SpriteCommonLoadTexture(spriteCommon_, 1, L"Resources/texture2.jpg", dxCommon_->GetDevice());
	sprite_->SpriteCommonLoadTexture(spriteCommon_, 2, L"Resources/texture3.jpg", dxCommon_->GetDevice());
	sprite_->SpriteCommonLoadTexture(spriteCommon_, 3, L"Resources/testTexture.png", dxCommon_->GetDevice());

	//�X�v���C�g�̐���
	titleSprite_ = titleSprite_.SpriteCreate(dxCommon_->GetDevice(), 1280, 720);

	//�e�N�X�`���ԍ��Z�b�g
	titleSprite_.SetTexNumber(3);
	//�e�N�X�`���T�C�Y�ݒ�
	titleSprite_.SetScale(XMFLOAT2(1280, 720));
	//���f
	titleSprite_.SpriteTransferVertexBuffer(titleSprite_);

	//�X�v���C�g�p�p�C�v���C�������Ăяo��
	PipelineSet spritePipelineSet = sprite_->SpriteCreateGraphicsPipeline(dxCommon_->GetDevice());

}

void GameScene::Update()
{
	player_->Update(matView, matProjection);

	//�摜�ύX
	titleSprite_.SetTexNumber(0);

	//�X�P�[���ύX�ƍX�V
	titleSprite_.SetScale(XMFLOAT2(100, 100));
	titleSprite_.SpriteTransferVertexBuffer(titleSprite_);

	//�|�W�V�����ύX�ƍX�V
	titleSprite_.SetPosition(XMFLOAT3(100, 100, 0));
	titleSprite_.SpriteUpdate(titleSprite_,spriteCommon_);
}

void GameScene::Draw()
{
	player_->Draw();

	//�X�v���C�g���ʃR�}���h
	sprite_->SpriteCommonBeginDraw(dxCommon_->GetCommandList(), spriteCommon_);

	titleSprite_.SpriteDraw(dxCommon_->GetCommandList(),titleSprite_,spriteCommon_, dxCommon_->GetDevice());

}
