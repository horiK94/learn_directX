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

//�p�x
float angle = 0;
//�p���x
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

	//���L�����̕\��
	//���[���h�ϊ�
	D3DXMATRIXA16 matWorld1, matWorld2;
	D3DXMatrixTranslation(&matWorld1, mx, 0.0f, mz);
	D3DXMatrixRotationY(&matWorld2, r);
	matWorld2 *= matWorld1;

	g_pd3DDeivece->SetTransform(D3DTS_WORLD, &matWorld2);
	RenderModel(hjikimodel);

	//�w�i�\��
	D3DXMATRIXA16 backMatrix, backScaleMatrix;
	D3DXMatrixTranslation(&backMatrix, 0.f, -1.0f, 0.0f);
	D3DXMatrixScaling(&backScaleMatrix, 1.3f, 1.0f, 1.2f);
	backScaleMatrix *= backMatrix;
	g_pd3DDeivece->SetTransform(D3DTS_WORLD, &backScaleMatrix);
	RenderModel(hbackmodel);

	//覐΂̏o��
	if (isTimerGoal(0) == TRUE)
	{
		AddComet();
		setTimer(0, 2000);
	}

	//覐΂̕\���A�ړ�
	for (int i = 0; i < MAXENEMY; i++)
	{
		if (enemys[i].isUsed == TRUE)
		{
			enemys[i].z -= 2.5f * loopTime;
			if (enemys[i].z < -7.5f)
			{
				//覐΂���ʊO�܂ňړ�����
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

	//�w�i
	hbackmodel = LoadModel(_T("back01.x"));
	if (hbackmodel == -1)
	{
		return E_FAIL;
	}

	//覐�
	hinsekimodel = LoadModel(_T("inseki.x"));
	if (hinsekimodel == -1)
	{
		return E_FAIL;
	}

	return S_OK;
}

void SetViews()
{
	//�����̐ݒ�
	g_pd3DDeivece->SetRenderState(D3DRS_AMBIENT, 0xfffffffff);

	//�r���[�ݒ�
	D3DXMATRIXA16 viewMatrix;
	D3DXVECTOR3 eyeVec{ 0, 13.0f, -5.0f };
	D3DXVECTOR3 atVec{ 0.0f, 0.0f, -1.0f };
	D3DXVECTOR3 upVec{ 0.0f, 1.0f, 0.0f };
	D3DXMatrixLookAtLH(&viewMatrix, &eyeVec, &atVec, &upVec);

	g_pd3DDeivece->SetTransform(D3DTS_VIEW, &viewMatrix);

	//�v���W�F�N�V����(�ˉe)�ϊ�
	D3DXMATRIXA16 proMatrix;
	D3DXMatrixPerspectiveFovLH(&proMatrix, D3DX_PI / 4, g_aspect, 1.0f, 100.0f);
	g_pd3DDeivece->SetTransform(D3DTS_PROJECTION, &proMatrix);

	//�G�z��̏�����
	ZeroMemory(&enemys, sizeof(Enemy) * MAXENEMY);

	//�^�C�}�[�w��
	setTimer(0, 3000);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	//�V�[�h�l�̐ݒ�
	srand(timeGetTime());
	if (SUCCEEDED(InitD3DWindow(_T("覐΃Q�[��"), 640, 480)))
	{
		//���f���̃��[�h
		if (FAILED(LoadModels()))
		{
			return 0;
		}
		//�����A�r���[�E�ˉe�ϊ��̐ݒ�
		SetViews();

		lastTime = timeGetTime();

		//���b�Z�[�W���[�v
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