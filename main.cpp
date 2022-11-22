#include "main.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	//�E�B���h�E����
	WinApp* win = nullptr;
	win = WinApp::GetInstance();
	win->CreateWindow_(L"DirectX");

	Masage* masage;	//���b�Z�[�W
	masage = Masage::GetInstance();

	//DirectX����������
	DirectXCommon* dx = nullptr;
	dx = DirectXCommon::GetInstance();
	dx->Initialize(win);

	//�L�[�{�[�h
	Input* input = nullptr;
	input = Input::GetInstance();
	input->Initialize(win);

	//�Q�[���V�[��
	GameScene* gameScene = nullptr;
	gameScene = new GameScene();
	gameScene->Initialize(dx, input);


	Pera* pera_ = new Pera();
	pera_->Initialize(dx->GetDevice(), dx);

#pragma endregion

	//�Q�[�����[�v
	while (true)
	{
		//���b�Z�[�W������H
		masage->Update();

#pragma region DirectX���t���[������

		//�L�[�{�[�h�X�V
		input->Update();

		gameScene->Update();


		dx->PeraPreDraw();

		// 4. �`��R�}���h
		gameScene->Draw();
		//pera_->Draw();

		dx->PeraPostDraw();


		dx->PreDraw();

		pera_->Draw();

		// 4. �`��R�}���h
		//gameScene->Draw();
		pera_->Draw();

		dx->PostDraw();


#pragma endregion


		//X�{�^���ŏI�����b�Z����������Q�[�����[�v�𔲂��� 
		if (masage->ExitGameloop() == 1)
		{
			break;
		}

		//�E�B���h�E�N���X��o�^����
		win->deleteWindow();
	}

	return 0;
}