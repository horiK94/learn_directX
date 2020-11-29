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
int hbakuhatsumodel = -1;

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

const float INSEKI_RADIUS = 1.0f;

enum { GM_MAIN, GM_OVER };
int gameMode = GM_MAIN;

int hgoFont = -1;

void GameMain();

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

void GameOver()
{
	//�����`��̂���GameMain()���Ăяo��
	GameMain();
	if (getPassedTime(1) < 2000)
	{
		float passedTime = getPassedTime(1);

		//�A���t�@�u�����f�B���O
		//�ʏ�̃����_�����O�ł̓��f���̐F�����̂܂܃o�b�N�o�b�t�@�̃s�N�Z���ɏ㏑�������	
		g_pd3DDeivece->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);		//�L����
		g_pd3DDeivece->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR);		// D3DRS_BLENDFACTOR�Őݒ肵���l���W��
		g_pd3DDeivece->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);		// 0xfffffffff����D3DRS_BLENDFACTOR�Őݒ肵���l���������␔���W��

		float a = 1.0f - passedTime / 2000;
		g_pd3DDeivece->SetRenderState(D3DRS_BLENDFACTOR, D3DCOLOR_COLORVALUE(a, a, a, a));

		//���@�̕\��
		D3DXMATRIXA16 transMatrix, rotateMatrix, jikiScaleMatrix;
		D3DXMatrixTranslation(&transMatrix, mx, 0.0f, mz);
		D3DXMatrixRotationY(&rotateMatrix, D3DXToRadian(angle));

		//�ő�1.4�{�܂ő傫���Ȃ�
		D3DXMatrixScaling(&jikiScaleMatrix, 1.0f + passedTime / 5000, 1.0f + passedTime / 5000, 1.0f + passedTime / 5000);

		jikiScaleMatrix = jikiScaleMatrix * rotateMatrix * transMatrix;

		g_pd3DDeivece->SetTransform(D3DTS_WORLD, &jikiScaleMatrix);
		RenderModel(hjikimodel);

		//�����̕\��
		D3DXMATRIXA16 bakuhatuRotateMatrix, bakuhatuScaleMatrix;
		D3DXMatrixRotationY(&bakuhatuRotateMatrix, timeGetTime() / 1000);
		D3DXMatrixScaling(&bakuhatuScaleMatrix, 1.0f + passedTime / 100, 1.0f + passedTime / 100, 1.0f + passedTime / 100);
		bakuhatuScaleMatrix = bakuhatuScaleMatrix * bakuhatuRotateMatrix * transMatrix;

		g_pd3DDeivece->SetTransform(D3DTS_WORLD, &bakuhatuScaleMatrix);
		RenderModel(hbakuhatsumodel);

		g_pd3DDeivece->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);		//������
	}
	if (getPassedTime(1) > 3000)
	{
		RECT rc = { 2, 162, 642, 262 };
		//Sprite�̕`��
		g_pTextSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);
		g_pxFont[hgoFont]->DrawTextW(g_pTextSprite, _T("GAME OVER"), -1, &rc, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(0.0f, 1.0f, 1.0f, 1.0f));

		SetRect(&rc, 0, 160, 640, 260);
		g_pxFont[hgoFont]->DrawTextW(g_pTextSprite, _T("GAME OVER"), -1, &rc, DT_CENTER | DT_VCENTER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f));
		g_pTextSprite->End();
	}
	if (getPassedTime(1) > 15000)
	{
		gameMode = GM_MAIN;
		mx = 0;
		mz = -2.0f;
		angle = 0;
		//�`��Ɋւ��鏉�������s��
		SetViews();
	}
}

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
	if (keys != NULL && gameMode == GM_MAIN)
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
	if (gameMode == GM_MAIN)
	{
		D3DXMATRIXA16 matWorld1, matWorld2;
		D3DXMatrixTranslation(&matWorld1, mx, 0.0f, mz);
		D3DXMatrixRotationY(&matWorld2, r);
		matWorld2 *= matWorld1;

		g_pd3DDeivece->SetTransform(D3DTS_WORLD, &matWorld2);
		RenderModel(hjikimodel);
	}

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

			//�Փ˔���
			if (pow(mx - enemys[i].x, 2) + pow(mz - enemys[i].z, 2) < INSEKI_RADIUS * INSEKI_RADIUS && gameMode == GM_MAIN)
			{
				//�Փ�
				//enemys[i].isUsed = FALSE;
				gameMode = GM_OVER;
				setTimer(1, 0);
			}
		}
	}
}

void Render()
{
	g_pd3DDeivece->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 255), 1, 0);

	if (SUCCEEDED(g_pd3DDeivece->BeginScene()))
	{
		switch (gameMode)
		{
		case GM_MAIN:
			GameMain();
			break;
		case GM_OVER:
			GameOver();
			break;
		}

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

	//����
	hbakuhatsumodel = LoadModel(_T("bakuha.x"));
	if (hbakuhatsumodel == -1)
	{
		return E_FAIL;
	}

	//font�̃��[�h
	hgoFont = CreateGameFont(_T("ARIAL"), 60, FW_BOLD);
	if (hgoFont == -1)
	{
		return E_FAIL;
	}

	g_pxFont[hgoFont]->PreloadText(_T("GAMEOVER"), 8);
	//g_pxFont[hgoFont]->PreloadCharacters(_T('A'), _T('Z'));

	return S_OK;
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