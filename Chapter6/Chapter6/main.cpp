#include "main.h"
#include  "Chapter6.h"

int modelIndex = -1;
float mx = 0.0f, mz = 0.0f;
DWORD lastTime = 0;
float loopTime = 0;
float moveSpeed = 5.0f;

void SetMatrices()
{
	const char* keys = GetKeyState();
	if (keys != NULL)
	{
		if (keys[DIK_UP] & 0x80)
		{
			mz += moveSpeed * loopTime;
		}
		if (keys[DIK_DOWN] & 0x80)
		{
			mz -= moveSpeed * loopTime;
		}

		if (keys[DIK_LEFT] & 0x80)
		{
			mx -= moveSpeed * loopTime;
		}
		if (keys[DIK_RIGHT] & 0x80)
		{
			mx += moveSpeed * loopTime;
		}
	}

	//ワールド変換
	D3DXMATRIXA16 matWorld1, matWorld2;
	D3DXMatrixTranslation(&matWorld1, mx, 0.0f, mz);
	D3DXMatrixRotationY(&matWorld2, D3DX_PI);
	matWorld2 *= matWorld1;

	g_pd3DDeivece->SetTransform(D3DTS_WORLD, &matWorld2);

	//ビュー変換
	D3DXMATRIXA16 viewMatrix;
	D3DXVECTOR3 eyeVec{ 0, 3.0f, -5.0f };
	D3DXVECTOR3 atVec{ 0.0f, 0.0f, 0.0f };
	D3DXVECTOR3 upVec{ 0.0f, 1.0f, 0.0f };
	D3DXMatrixLookAtLH(&viewMatrix, &eyeVec, &atVec, &upVec);

	g_pd3DDeivece->SetTransform(D3DTS_VIEW, &viewMatrix);

	//プロジェクション変換
	D3DXMATRIXA16 proMatrix;
	D3DXMatrixPerspectiveFovLH(&proMatrix, D3DX_PI / 4, g_aspect, 1.0f, 100.0f);
	g_pd3DDeivece->SetTransform(D3DTS_PROJECTION, &proMatrix);
}

void Render()
{
	g_pd3DDeivece->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 255), 1, 0);

	if (SUCCEEDED(g_pd3DDeivece->BeginScene()))
	{
		g_pd3DDeivece->SetRenderState(D3DRS_AMBIENT, 0xffffffff);

		SetMatrices();

		RenderModel(modelIndex);
		g_pd3DDeivece->EndScene();
	}

	g_pd3DDeivece->Present(NULL, NULL, NULL, NULL);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	if (SUCCEEDED(InitD3DWindow(_T("Chapter5 Window"), 640, 480)))
	{
		modelIndex = LoadModel(_T("catsenkan.x"));
		if (modelIndex == -1)return 0;

		lastTime = timeGetTime();

		//メッセージループ
		MSG msg = { 0 };
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				DWORD currentTime = timeGetTime();
				loopTime = (currentTime - lastTime) / 1000.0f;
				lastTime = currentTime;
				Render();
			}
		}
	}

	UnregisterClass(_T("Chapter5 Window"), GetModuleHandle(NULL));
	return 0;
}