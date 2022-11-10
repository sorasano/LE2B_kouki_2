#include "Masage.h"


Masage* Masage::GetInstance()
{
	static Masage instance;
	return &instance;
}

void Masage::Update()
{
	//���b�Z�[�W������H
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);	//�L�[���̓��b�Z�[�W�̏���
		DispatchMessage(&msg);	//�v���V�[�W���Ƀ��b�Z�[�W�𑗂�
	}
}

bool Masage::ExitGameloop()
{
	//X�{�^���ŏI�����b�Z����������Q�[�����[�v�𔲂��� 
	if (msg.message == WM_QUIT)
	{
		IDXGIDebug* giDebugInterface = nullptr;

		if (giDebugInterface == nullptr)
		{
			//�쐬
			typedef HRESULT(__stdcall* fPtr)(const IID&, void**);
			HMODULE hDll = GetModuleHandleW(L"dxgidebug.dll");
			if (hDll != 0)
			{
				fPtr DXGIGetDebugInterface =
					(fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");

				DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&giDebugInterface);

				//�o��
				giDebugInterface->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_DETAIL);
			}
		}
		return 1;
	}
}
