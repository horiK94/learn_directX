#include "main.h"
#include  "Chapter7.h"

struct Enemy
{
	float x, z;
	BOOL isUsed;
};

int hjikimodel = -1;

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

D3DXVECTOR3 myPos(0.0f, 0.0f, 0.0f);
D3DXVECTOR3 myAngle(0.0f, 0.f, 0.0f);
float angleSpeed = D3DX_PI / 2;

void GameMain();

void SetViews()
{
	//環境光の設定
	g_pd3DDeivece->SetRenderState(D3DRS_AMBIENT, 0xfffffffff);

	//プロジェクション(射影)変換
	D3DXMATRIXA16 proMatrix;
	D3DXMatrixPerspectiveFovLH(&proMatrix, D3DX_PI / 4, g_aspect, 1.0f, 100.0f);
	g_pd3DDeivece->SetTransform(D3DTS_PROJECTION, &proMatrix);

	//敵配列の初期化
	ZeroMemory(&enemys, sizeof(Enemy) * MAXENEMY);

	//タイマー指導
	setTimer(0, 3000);
}

void GameMain()
{
	D3DXVECTOR3 myVec(0.0f, 0.0f, 0.0f);

	const char* keys = GetKeyState();
	if(keys != NULL)
	{
		if (keys[DIK_UP] & 0x80)
		{
			myVec.z = moveSpeed * loopTime;
		}
		if (keys[DIK_DOWN] & 0x80)
		{
			myVec.z = -moveSpeed * loopTime;
		}

		if (keys[DIK_LEFT] & 0x80)
		{
			myAngle.y -= moveSpeed * loopTime;
		}
		if (keys[DIK_RIGHT] & 0x80)
		{
			myAngle.y += moveSpeed * loopTime;
		}

		if(myAngle.y < 0)
		{
			myAngle.y += D3DX_PI * 2;
		}
		else if(myAngle.y >= D3DX_PI * 2)
		{
			myAngle.y -= D3DX_PI * 2;
		}
	}
	
	//自キャラの表示
	D3DXMATRIXA16 matWorld1, matWorld2;
	D3DXMatrixRotationY(&matWorld2, myAngle.y);

	D3DXVECTOR3 myVec2;
	D3DXVec3TransformCoord(&myVec2, &myVec, &matWorld2);
	myPos += myVec2;

	//ワールド変換
	D3DXMatrixTranslation(&matWorld1, myPos.x, myPos.y, myPos.z);
	D3DXMATRIXA16 matWorld3 =  matWorld2 * matWorld1;

	g_pd3DDeivece->SetTransform(D3DTS_WORLD, &matWorld3);
	RenderModel(hjikimodel);

	//ビュー設定
	D3DXVECTOR3 viewVecEye(0.0f, 1.0f, -5.0f);		//カメラの位置
	D3DXVECTOR3 viewVecLook(0.0f, 2.0f, 4.0f);		//向き先
	D3DXVec3TransformCoord(&viewVecEye, &viewVecEye, &matWorld2);		//原点をもとに回転した場合の位置を求める
	//移動も考慮したeyeの値求める
	D3DXVECTOR3 eyeVec(viewVecEye.x + myPos.x, viewVecEye.y + myPos.y, viewVecEye.z + myPos.z);

	//原点をもとに回転した場合の向き先を求める
	D3DXVec3TransformCoord(&viewVecLook, &viewVecLook, &matWorld2);
	D3DXVECTOR3 lookVec(viewVecLook.x + myPos.x, viewVecLook.y + myPos.y, viewVecLook.z + myPos.z);
	
	D3DXVECTOR3 upVec{ 0.0f, 1.0f, 0.0f };
	D3DXMATRIXA16 cameraMatrix;
	D3DXMatrixLookAtLH(&cameraMatrix, &eyeVec, &lookVec, &upVec);

	g_pd3DDeivece->SetTransform(D3DTS_VIEW, &cameraMatrix);
}

void Render()
{
	g_pd3DDeivece->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 255), 1, 0);

	if (SUCCEEDED(g_pd3DDeivece->BeginScene()))
	{
		GameMain();
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
	
	return S_OK;
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	//シード値の設定
	srand(timeGetTime());
	if (SUCCEEDED(InitD3DWindow(_T("かくれんぼ"), 640, 480)))
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