#include "main.h"
#include "Chapter7.h"

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
float angleSp = D3DX_PI / 2;		//90度/s

const float SENKAN_SIZE = 1.0f;

D3DXVECTOR3 myPos(0.0f, 1.0f, 0.0f);
D3DXVECTOR3 myAngle(0.0f, 0.f, 0.0f);
float angleSpeed = D3DX_PI / 2;

//敵データ
int hteki1model, hteki2model;
D3DXVECTOR3 tekiPos(-2.0f, 1.0f, -2.0f);
float tekiSpeed = 3.0f;
float tekiAngle = 0.0f;

enum
{
	TK_CHASE,
	TK_SEARCH,
};

int tekiMode = TK_SEARCH;
float searchPivot = 0.0f;		//自キャラが見つからないときの中心角度
int searchTurn = 1;		//1: 時計回り, -1: 反時計回り(自キャラが見つからないときの見回り向き)

void GameMain();

void SetViews()
{
	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 0.5f;
	light.Diffuse.b = 0.0f;

	D3DXVECTOR3 dirVec = D3DXVECTOR3(2, 2, -2);
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &dirVec);
	light.Range = 1000.0f;

	g_pd3DDeivece->SetLight(0, &light);
	g_pd3DDeivece->LightEnable(0, TRUE);
	g_pd3DDeivece->SetRenderState(D3DRS_LIGHTING, TRUE);

	//環境光の設定
	g_pd3DDeivece->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_COLORVALUE(0.5f, 0.3f, 0.33f, 1.0f));

	//プロジェクション(射影)変換
	D3DXMATRIXA16 proMatrix;
	D3DXMatrixPerspectiveFovLH(&proMatrix, D3DX_PI / 4, g_aspect, 1.0f, 100.0f);
	g_pd3DDeivece->SetTransform(D3DTS_PROJECTION, &proMatrix);

	//敵始動
	setTimer(2, 0);
}

//時期の衝突判定
//pFoundagley: あたった場合、あたった角度を返す
BOOL ProbeJiki(float* pFoundagley)
{
	//自キャラの境界ボックス作成
	D3DXVECTOR3 mMin = D3DXVECTOR3(myPos.x - SENKAN_SIZE / 2, myPos.y - SENKAN_SIZE / 2, myPos.z - SENKAN_SIZE / 2);
	D3DXVECTOR3 mMax = D3DXVECTOR3(myPos.x + SENKAN_SIZE / 2, myPos.y + SENKAN_SIZE / 2, myPos.z + SENKAN_SIZE / 2);

	D3DXVECTOR3 probeVecBase = D3DXVECTOR3(0, 0, 1.0);

	//45度の視野を4.5度単位で探索
	for (int i = -5; i < 6; i++)
	{
		D3DXVECTOR3 probeVec;
		D3DXMATRIXA16 rotateMatrix;
		D3DXMatrixRotationY(&rotateMatrix, tekiAngle + D3DX_PI / 40.0f * i);
		D3DXVec3TransformCoord(&probeVec, &probeVecBase, &rotateMatrix);

		if (D3DXBoxBoundProbe(&mMin, &mMax, &tekiPos, &probeVec))
		{
			*pFoundagley = tekiAngle + D3DX_PI / 40.0f * i;
			return TRUE;
		}
	}

	return FALSE;
}

void MoveTeki()
{
	//ワールド変換行列作成
	D3DXMATRIXA16 matWorld1, matWorld2, matWorld3;
	D3DXMatrixTranslation(&matWorld1, tekiPos.x, 1.6f, tekiPos.z);
	D3DXMatrixRotationY(&matWorld2, tekiAngle);
	matWorld3 = matWorld2 * matWorld1;

	g_pd3DDeivece->SetTransform(D3DTS_WORLD, &matWorld3);

	DWORD passedTime = getPassedTime(2);
	if (passedTime < 90000)
	{
		//攻撃モード
		RenderModel(hteki1model);

		switch (tekiMode)
		{
		case TK_SEARCH:
		{
			//loopTimeに応じた角速度分の角度変更
			tekiAngle += searchTurn * angleSp * loopTime;
			if (fabs(tekiAngle - searchPivot) > D3DX_PI / 1.5f)
			{
				//120度見たら向きを変えて240度。そしたら向きを変えて240度...となるように動くため、
				//fabs(絶対値関数)が120度(=2/3πラジアン)を超えたら向きを変える
				searchTurn *= -1;
			}
			float jikiAngle;
			if (ProbeJiki(&jikiAngle))
			{
				//見つけた
				tekiMode = TK_CHASE;
			}
		}
		break;
		case TK_CHASE:
			break;
		}
	}
	if (passedTime >= 90000)
	{
		//攻撃モードより見た目小さいため、y座標を低く設定する
		D3DXMatrixTranslation(&matWorld1, tekiPos.x, 1.0f, tekiPos.z);
		matWorld2 *= matWorld1;
		g_pd3DDeivece->SetTransform(D3DTS_WORLD, &matWorld2);

		//弱体モード
		RenderModel(hteki2model);
	}
	if (passedTime >= 110000)
	{
		setTimer(2, 0);
	}

	//変身
	if (passedTime >= 90000 && passedTime < 95000)
	{
		g_pd3DDeivece->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

		float alpha = 1 - (passedTime - 90000) / 5000.0f;
		g_pd3DDeivece->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVBLENDFACTOR);
		g_pd3DDeivece->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR);
		g_pd3DDeivece->SetRenderState(D3DRS_BLENDFACTOR, D3DCOLOR_COLORVALUE(alpha, alpha, alpha, alpha));

		g_pd3DDeivece->SetTransform(D3DTS_WORLD, &matWorld3);
		RenderModel(hteki1model);

		g_pd3DDeivece->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}
	if (passedTime >= 105000)
	{
		g_pd3DDeivece->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

		float alpha = 1 - (passedTime - 105000) / 5000.0f;
		g_pd3DDeivece->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_BLENDFACTOR);
		g_pd3DDeivece->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVBLENDFACTOR);
		g_pd3DDeivece->SetRenderState(D3DRS_BLENDFACTOR, D3DCOLOR_COLORVALUE(alpha, alpha, alpha, alpha));

		g_pd3DDeivece->SetTransform(D3DTS_WORLD, &matWorld3);
		RenderModel(hteki1model);

		g_pd3DDeivece->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}
}

BOOL hitTikei(D3DXVECTOR3* pMinA, D3DXVECTOR3* pMaxA)
{
	for (int i = 0; i < MAX_BUILDING; i++)
	{
		D3DXVECTOR3* pMinB, * pMaxB;
		pMinB = &boundingBoxes[i].minVec;
		pMaxB = &boundingBoxes[i].maxVec;

		if (pMinA->x > pMaxB->x)
		{
			continue;
		}

		if (pMaxA->x < pMinB->x)
		{
			continue;
		}

		if (pMinA->y > pMaxB->y)
		{
			continue;
		}

		if (pMaxA->y < pMinB->y)
		{
			continue;
		}

		if (pMinA->z > pMaxB->z)
		{
			continue;
		}

		if (pMaxA->z < pMinB->z)
		{
			continue;
		}

		return TRUE;
	}
	return FALSE;
}

void GameMain()
{
	D3DXVECTOR3 myVec(0.0f, 0.0f, 0.0f);

	const char* keys = GetKeyState();
	if (keys != NULL)
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

		if (myAngle.y < 0)
		{
			myAngle.y += D3DX_PI * 2;
		}
		else if (myAngle.y >= D3DX_PI * 2)
		{
			myAngle.y -= D3DX_PI * 2;
		}
	}

	//自キャラの表示
	D3DXMATRIXA16 matWorld1, matWorld2;
	D3DXMatrixRotationY(&matWorld2, myAngle.y);

	D3DXVECTOR3 myVec2;
	D3DXVec3TransformCoord(&myVec2, &myVec, &matWorld2);

	//地形にぶつかる等で進めない場合は位置を足さない
	D3DXVECTOR3 willMovePos = myPos + myVec2;
	D3DXVECTOR3 pMinA(willMovePos.x - SENKAN_SIZE / 2.0f, willMovePos.y - SENKAN_SIZE / 2.0f, willMovePos.z - SENKAN_SIZE / 2.0f);
	D3DXVECTOR3 pMaxA(willMovePos.x + SENKAN_SIZE / 2.0f, willMovePos.y + SENKAN_SIZE / 2.0f, willMovePos.z + SENKAN_SIZE / 2.0f);
	if (!hitTikei(&pMinA, &pMaxA))
	{
		myPos = willMovePos;
	}

	//ワールド変換
	D3DXMatrixTranslation(&matWorld1, myPos.x, myPos.y, myPos.z);
	D3DXMATRIXA16 matWorld3 = matWorld2 * matWorld1;

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

	//壁の描画
	for (int i = 0; i < MAX_BUILDING; i++)
	{
		D3DXMatrixTranslation(&matWorld1, buildings[i].pos.x, buildings[i].pos.y, buildings[i].pos.z);
		g_pd3DDeivece->SetTransform(D3DTS_WORLD, &matWorld1);

		RenderModel(buildings[i].hmodel);
	}

	MoveTeki();
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
	//敵モデル読み込み
	hteki1model = LoadModel(_T("teki1.x"));
	if (hteki1model == -1)
	{
		return E_FAIL;
	}
	hteki2model = LoadModel(_T("teki2.x"));
	if (hteki2model == -1)
	{
		return E_FAIL;
	}

	hjikimodel = LoadModel(_T("catsenkan.x"));
	if (hjikimodel == -1)
	{
		return E_FAIL;
	}

	for (int i = 0; i < MAX_BUILDING; i++)
	{
		int h = LoadModel(buildings[i].xname);
		if (h == -1)
		{
			return E_FAIL;
		}
		buildings[i].hmodel = h;

		//境界ボックスの作成
		//頂点数の取得
		DWORD numVector = g_models[h].pMesh->GetNumVertices();
		//頂点データ間のバイト数
		DWORD vertexSize = D3DXGetFVFVertexSize(g_models[h].pMesh->GetFVF());

		//頂点バッファ
		LPDIRECT3DVERTEXBUFFER9 pvb;
		HRESULT result = g_models[h].pMesh->GetVertexBuffer(&pvb);

		if (FAILED(result))
		{
			return E_FAIL;
		}

		//頂点バッファをLockメソッドでロックすると、頂点データのアドレスが取得できる
		BYTE* pvetices;
		result = pvb->Lock(0, 0, (VOID**)&pvetices, 0);
		if (FAILED(result))
		{
			return E_FAIL;
		}

		//バウンディングボックスの取得
		D3DXVECTOR3 boundingMinVec, boundingMaxVec;
		result = D3DXComputeBoundingBox((D3DXVECTOR3*)pvetices, numVector, vertexSize, &boundingMinVec, &boundingMaxVec);
		if (FAILED(result))
		{
			return E_FAIL;
		}

		//オブジェクト座標からワールド座標時(回転はしていないのでそのままワールド座標を足している)
		boundingBoxes[i].minVec = buildings[i].pos + boundingMinVec;
		boundingBoxes[i].maxVec = buildings[i].pos + boundingMaxVec;
		//頂点バッファのアンロック
		pvb->Unlock();
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