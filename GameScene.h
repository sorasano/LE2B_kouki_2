#include "DirectXCommon.h"
#include "input.h"
#include "DirectXTex.h"
#include "Cube.h"
#include "object3D.h"
#include "list"
#include "memory"
#include "Texture.h"
#include "Model.h"
#include "Object3D.h"
#include "Player.h"
#include "Sound.h"
#include "Sprite.h"
#include "Pera.h"

class GameScene
{
	//メンバ関数
public:
	GameScene();
	~GameScene();
	void Initialize(DirectXCommon* dxCommon, Input* input);
	void Update();
	void Draw();

	//メンバ変数
private: 
	//デバイスとinput
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;

	//プレイヤーのモデル
	std::unique_ptr<Model> playerModel_;

	//プレイヤー
	std::unique_ptr<Player> player_;

	//オブジェクト
	std::unique_ptr<Object3D> object3Ds_;

	//スプライト
	Sprite* sprite_ = new Sprite;

	//スプライト共通データ生成
	SpriteCommon spriteCommon_;

	//タイトル
	Sprite titleSprite_;

	Pera *pera_ = new Pera;

	//射影変換
	XMMATRIX matProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f),			//上下画角45度
		(float)window_width / window_height,//アスペクト比(画面横幅/画面立幅)
		0.1f, 1000.0f						//前端、奥端
	);

	//ビュー変換行列
	XMMATRIX matView;
	XMFLOAT3 eye = { -10, 10, 30 };
	XMFLOAT3 target = { 0, 0, 0 };
	XMFLOAT3 up = { 0, 1, 0 };
};
