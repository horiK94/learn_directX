#include "main.h"
#include  "Chapter6.h"

struct Enemy
{
	float x, z;
	BOOL isUsed;
};

int hjikimodel = -1;
int hbackmodel = -1;
int hinsekimodel = -1;
float mx = 0.0f, mz = 0.0f;
DWORD lastTime = 0;
float loopTime = 0;
float moveSpeed = 5.0f;

//角度
float angle = 0;
//角速度
float angleSp = 90;

const int MAXENEMY = 50;
Enemy enemys[MAXENEMY];

void AddComet()
{
	for (int i = 0; i < MAXENEMY; i++)
	{
		if (enemys[i].isUsed == FALSE)
		{
			enemys[i].x = (float)(rand() % 29) / 2 - 7;
			enemys[i].z = 7;

			enemys[i].isUsed = TRUE;
			break;
		}
	}
}

void GameMain()
{
	const char* keys = GetKeyState();
	float vectPow = 0;
	if (keys != NULL)
	{
		if (keys[DIK_UP] & 0x80)
		{
			vectPow = moveSpeed * loopTime;
		}
		if (keys[DIK_DOWN] & 0x80)
		{
			vectPow = -moveSpeed * loopTime;
		}

		if (keys[DIK_LEFT] & 0x80)
		{
			angle = angle - angleSp * loopTime;
		}
		if (keys[DIK_RIGHT] & 0x80)
		{
			angle = angle + angleSp * loopTime;
		}

		if (angle < 0)
		{
			angle += 360;
		}
		if (angle > 360)
		{
			angle -= 360;
		}
	}
	float r = D3DXToRadian(angle);

	mx += vectPow * sinf(r);
	mz += vectPow * cosf(r);

	if (mx < -7)
	{
		mx = -7;
	}
	else if (mx > 7)
	{
		mx = 7;
	}

	if (mz < -6)
	{
		mz = -6;
	}
	else if (mz > 6)
	{
		mz = 6;
	}

	//自キャラの表示
	//ワールド変換
	D3DXMATRIXA16 matWorld1, matWorld2;
	D3DXMatrixTranslation(&matWorld1, mx, 0.0f, mz);
	D3DXMatrixRotationY(&matWorld2, r);
	matWorld2 *= matWorld1;

	g_pd3DDeivece->SetTransform(D3DTS_WORLD, &matWorld2);
	RenderModel(hjikimodel);

	//背景表示
	D3DXMATRIXA16 backMatrix, backScaleMatrix;
	D3DXMatrixTranslation(&backMatrix, 0.f, -1.0f, 0.0f);
	D3DXMatrixScaling(&backScaleMatrix, 1.3f, 1.0f, 1.2f);
	backScaleMatrix *= backMatrix;
	g_pd3DDeivece->SetTransform(D3DTS_WORLD, &backScaleMatrix);
	RenderModel(hbackmodel);

	//隕石の出現
	if (isTimerGoal(0) == TRUE)
	{
		AddComet();
		setTimer(0, 2000);
	}

	//隕石の表示、移動
	for (int i = 0; i < MAXENEMY; i++)
	{
		if (enemys[i].isUsed == TRUE)
		{
			enemys[i].z -= 2.5f * loopTime;
			if (enemys[i].z < -7.5f)
			{
				//隕石が画面外まで移動した
				enemys[i].isUsed = FALSE;
			}
			D3DXMATRIXA16 transMatrix, rotateMatrix;
			D3DXMatrixTranslation(&transMatrix, enemys[i].x, 0, enemys[i].z);
			D3DXMatrixRotationY(&rotateMatrix, timeGetTime() / 1000.0f);

			rotateMatrix *= transMatrix;
			
			g_pd3DDeivece->SetTransform(D3DTS_WORLD, &rotateMatrix);
			RenderModel(hinsekimodel);
		}
	}
}

void Render()
{
	g_pd3DDeivece->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 255), 1, 0);

	if (SUCCEEDED(g_pd3DDeivece->BeginScene()))
	{
		g_pd3DDeivece->SetRenderState(D3DRS_AMBIENT, 0xffffffff);

		GameMain();

		//RenderModel(hjikimodel);
		g_pd3DDeivece->EndScene();
	}

	g_pd3DDeivece->Present(NULL, NULL, NULL, NULL);
}

HRESULT LoadModels()
{
	hjikimodel = LoadModel(_T("catsenkan.x"));
	if (hjikimodel == -1)
	{
		return E_FAIL;
	}

	//背景
	hbackmodel = LoadModel(_T("back01.x"));
	if (hbackmodel == -1)
	{
		return E_FAIL;
	}

	//隕石
	hinsekimodel = LoadModel(_T("inseki.x"));
	if (hinsekimodel == -1)
	{
		return E_FAIL;
	}

	return S_OK;
}

void SetViews()
{
	//環境光の設定
	g_pd3DDeivece->SetRenderState(D3DRS_AMBIENT, 0xfffffffff);

	//ビュー設定
	D3DXMATRIXA16 viewMatrix;
	D3DXVECTOR3 eyeVec{ 0, 13.0f, -5.0f };
	D3DXVECTOR3 atVec{ 0.0f, 0.0f, -1.0f };
	D3DXVECTOR3 upVec{ 0.0f, 1.0f, 0.0f };
	D3DXMatrixLookAtLH(&viewMatrix, &eyeVec, &atVec, &upVec);

	g_pd3DDeivece->SetTransform(D3DTS_VIEW, &viewMatrix);

	//プロジェクション(射影)変換
	D3DXMATRIXA16 proMatrix;
	D3DXMatrixPerspectiveFovLH(&proMatrix, D3DX_PI / 4, g_aspect, 1.0f, 100.0f);
	g_pd3DDeivece->SetTransform(D3DTS_PROJECTION, &proMatrix);

	//敵配列の初期化
	ZeroMemory(&enemys, sizeof(Enemy) * MAXENEMY);

	//タイマー指導
	setTimer(0, 3000);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	//シード値の設定
	srand(timeGetTime());
	if (SUCCEEDED(InitD3DWindow(_T("隕石ゲーム"), 640, 480)))
	{
		//モデルのロード
		if (FAILED(LoadModels()))
		{
			return 0;
		}
		//環境光、ビュー・射影変換の設定
		SetViews();

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