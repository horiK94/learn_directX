#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#include <tchar.h>

#include "Chapter5.h"

//グローバル変数
LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3DDeivece = NULL;

float g_aspect = 1.0f;

void CleanUp()
{
	if (g_pd3DDeivece != NULL)
	{
		g_pd3DDeivece->Release();
	}

	if (g_pD3D != NULL)
	{
		g_pD3D->Release();
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
		CleanUp();
		PostQuitMessage(0);		//WM_QUITメッセージを送る → Windowsを通してメッセージループに渡り、WinMain関数のメッセージループを抜ける
		return 0;
	}

	//自作のウィンドウプロシージャルで処理しないイベントメッセージはDefWindowProcに任せれば、ウィンドウサイズ変更などをしてくれる
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HRESULT InitD3DWindow(LPCTSTR winTitle, int w, int h)
{
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
		_T("Chapter5"),
		NULL
	};
	RegisterClassEx(&wc);

	//ウィンドウ作成
	HWND hwnd = CreateWindow(
		_T("Chapter5"),
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
	g_aspect = (float)w / h;

	//D3D9の作成
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	//D3Dデバイスの作成
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
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

	return S_OK;
}