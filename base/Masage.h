#pragma once
#include "dxgidebug.h"
#include <d3d12.h>

class Masage
{
public:
	static Masage* GetInstance();
	void Update();
	//X�{�^���ŏI�����b�Z����������Q�[�����[�v�𔲂��� 
	bool ExitGameloop();
public:
	MSG msg{};
};

