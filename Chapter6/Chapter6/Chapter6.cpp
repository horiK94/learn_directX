#include "Chapter6.h"

//グローバル変数
//他のソースコードでも使用できるようにexturnをつける
LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3DDeivece = NULL;

const int MAX_MODEL = 64;
Model g_models[MAX_MODEL];

float g_aspect = 1.0f;

LPDIRECTINPUT8 g_pDI = NULL;
LPDIRECTINPUTDEVICE8 g_pDIDevice = NULL;

char g_keys[256];

const int MAX_TIMER = 16;
DWORD g_goadtimes[MAX_TIMER];

void setTimer(int idx, DWORD time)
{
	if(idx >= MAX_TIMER)
	{
		return;
	}

	g_goadtimes[idx] = timeGetTime() + time;
}

BOOL isTimerGoal(int idx)
{
	return g_goadtimes[idx] <= timeGetTime();
}

//経過時間を取得する
DWORD pgetPassedTime(int idx)
{
	return timeGetTime() - g_goadtimes[idx];
}

void CleanD3D()
{
	for (int i = 0; i < MAX_MODEL; i++)
	{
		if (g_models[i].used == FALSE)
		{
			continue;
		}

		if (g_models[i].pMesh != NULL)
		{
			g_models[i].pMesh->Release();
		}

		if (g_models[i].pMaterial != NULL)
		{
			delete[] g_models[i].pMaterial;
		}

		if (g_models[i].pTexture != NULL)
		{
			for (int j = 0; j < g_models[i].numMaterials; j++)
			{
				g_models[i].pTexture[j]->Release();
			}
			delete[] g_models[i].pTexture;
		}
	}

	if (g_pd3DDeivece != NULL)
	{
		g_pd3DDeivece->Release();
	}
	if (g_pD3D != NULL)
	{
		g_pD3D->Release();
	}

	if (g_pDIDevice != NULL)
	{
		g_pDIDevice->Unacquire();
		g_pDIDevice->Release();
	}
	if (g_pDI != NULL)
	{
		g_pDI->Release();
	}
}

//ウィンドウプロシージャ. 関数名は自由だが、引数と返り値は指定あり
LRESULT WINAPI MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//msgにはイベントを表す定数が入っている
	switch (msg)
	{
	case WM_DESTROY:		//ウィンドウを閉じるとき
		//特に問題がないことを伝えるために0
		CleanD3D();
		PostQuitMessage(0);		//WM_QUITメッセージを送る → Windowsを通してメッセージループに渡り、WinMain関数のメッセージループを抜ける
		return 0;
	}

	//自作のウィンドウプロシージャルで処理しないイベントメッセージはDefWindowProcに任せれば、ウィンドウサイズ変更などをしてくれる
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HRESULT InitD3DWindow(LPCTSTR winTitle, int w, int h)
{
	ZeroMemory(&g_models, sizeof(Model) * MAX_MODEL);

	//ウィンドウクラス作成
	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		MsgProc,
		0L,
		0L,
		GetModuleHandle(NULL),
		NULL,
		NULL,
		NULL,
		NULL,
		_T("Chapter5 Window"),
		NULL
	};
	RegisterClassEx(&wc);

	//ウィンドウ作成
	HWND hwnd = CreateWindow(
		_T("Chapter5 Window"),
		winTitle,
		WS_OVERLAPPED | WS_SYSMENU,
		100,
		100,
		w,
		h,
		NULL,
		NULL,
		wc.hInstance,
		NULL);

	//ビューポートのアスペクト比を求める
	g_aspect = (float)w / (float)h;

	//D3D9の作成
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	//D3Dデバイスの作成
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3DDeivece)))
	{
		if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3DDeivece)))
		{
			return E_FAIL;
		}
	}

	//zバッファをonに
	g_pd3DDeivece->SetRenderState(D3DRS_ZENABLE, TRUE);
	ShowWindow(hwnd, SW_SHOWDEFAULT);

	//DirectInputの初期化
	if (FAILED(DirectInput8Create(wc.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&g_pDI, NULL)))
	{
		return E_FAIL;
	}
	if (FAILED(g_pDI->CreateDevice(GUID_SysKeyboard, &g_pDIDevice, NULL)))
	{
		return E_FAIL;
	}

	g_pDIDevice->SetDataFormat(&c_dfDIKeyboard);
	g_pDIDevice->SetCooperativeLevel(hwnd,
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	return S_OK;
}

int LoadModel(LPCTSTR fileName)
{
	//未使用の要素を探す
	int idx = MAX_MODEL;
	for (int i = 0; i < MAX_MODEL; i++)
	{
		if (g_models[i].used == FALSE)
		{
			idx = i;
			break;
		}
	}

	if (idx >= MAX_MODEL)
	{
		return -1;
	}

	LPD3DXBUFFER temporaryBuffer = NULL;

	if (FAILED(D3DXLoadMeshFromX(fileName, D3DXMESH_SYSTEMMEM, g_pd3DDeivece, NULL, &temporaryBuffer, NULL, &g_models[idx].numMaterials, &g_models[idx].pMesh)))
	{
		MessageBox(NULL, _T("Xファイルが見つかりません"), _T("3D Lib"), MB_OK);
		return -1;
	}

	D3DXMATERIAL* d3dxMaterial = (D3DXMATERIAL*)temporaryBuffer->GetBufferPointer();
	DWORD materialNum = g_models[idx].numMaterials;
	g_models[idx].pMaterial = new D3DMATERIAL9[materialNum];
	if (g_models[idx].pMaterial == NULL)
	{
		return -1;
	}

	g_models[idx].pTexture = new LPDIRECT3DTEXTURE9[materialNum];
	if (g_models[idx].pTexture == NULL)
	{
		return -1;
	}

	for (int i = 0; i < materialNum; i++)
	{
		g_models[idx].pMaterial[i] = d3dxMaterial[i].MatD3D;

		g_models[idx].pMaterial[i].Ambient = d3dxMaterial[i].MatD3D.Diffuse;

		//テクスチャの読み込み
		g_models[idx].pTexture[i] = NULL;
		if (d3dxMaterial[i].pTextureFilename != NULL && lstrlenA(d3dxMaterial[i].pTextureFilename) > 0)
		{
			//D3DXLoadMeshFromXが返す変数のテクスチャファイル名はMBCSのLPSTR
			//D3DXCreateTextureFromFileに渡すとコンパイルエラーになるので、
			//(LPCSTR)d3dxMaterial[i].pTextureFilename として入れたくなるが、
			//実際のデータはMBCSなのでエラーになってしまう
			//D3DXCreateTextureFromFileA関数を呼び、MBCS関数を呼ぶことで回避している
			if (FAILED(D3DXCreateTextureFromFileA(g_pd3DDeivece, d3dxMaterial[i].pTextureFilename, &g_models[idx].pTexture[i])))
			{
				MessageBox(NULL, _T("テクスチャが見つかりません"), _T("3D Lib"), MB_OK);
				return -1;
			}
		}
	}

	temporaryBuffer->Release();
	g_models[idx].used = true;
	return idx;
}



void RenderModel(int idx)
{
	for (int i = 0; i < g_models[idx].numMaterials; i++)
	{
		g_pd3DDeivece->SetMaterial(&g_models[idx].pMaterial[i]);
		g_pd3DDeivece->SetTexture(0, g_models[idx].pTexture[i]);

		g_models[idx].pMesh->DrawSubset(i);
	}
}

const char* GetKeyState()
{
	HRESULT result = g_pDIDevice->Acquire();
	if (result == DI_OK || result == S_FALSE)
	{
		g_pDIDevice->GetDeviceState(sizeof(g_keys), &g_keys);
		return g_keys;
	}
	return NULL;
}