#include "main.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	//ウィンドウ生成
	WinApp* win = nullptr;
	win = WinApp::GetInstance();
	win->CreateWindow_(L"DirectX");

	Masage* masage;	//メッセージ
	masage = Masage::GetInstance();

	//DirectX初期化処理
	DirectXCommon* dx = nullptr;
	dx = DirectXCommon::GetInstance();
	dx->Initialize(win);

	//キーボード
	Input* input = nullptr;
	input = Input::GetInstance();
	input->Initialize(win);

	//ゲームシーン
	GameScene* gameScene = nullptr;
	gameScene = new GameScene();
	gameScene->Initialize(dx, input);

#pragma endregion

	//ゲームループ
	while (true)
	{
		//メッセージがある？
		masage->Update();

#pragma region DirectX毎フレーム処理

		//キーボード更新
		input->Update();

		gameScene->Update();

		dx->PreDraw();
		// 4. 描画コマンド
		gameScene->Draw();


		dx->PostDraw();

#pragma endregion


		//Xボタンで終了メッセ時が来たらゲームループを抜ける 
		if (masage->ExitGameloop() == 1)
		{
			break;
		}

		//ウィンドウクラスを登録解除
		win->deleteWindow();
	}

	return 0;
}