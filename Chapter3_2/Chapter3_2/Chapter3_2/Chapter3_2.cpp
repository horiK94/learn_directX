//-----------------------------------------------------------------------------
// File: CreateDevice.cpp
//
// Desc: This is the first tutorial for using Direct3D. In this tutorial, all
//       we are doing is creating a Direct3D device and using it to clear the
//       window.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <d3d9.h>
/*
#pragma warning( disable : 4996 ) 
コンパイル時に指定した番号の警告を出さないようにしろ というのをコンパイラに命令している

#pragma warning( default : 4996 )
コンパイル時に指定した番号の警告を出すようにしろ(有効) というのをコンパイラに命令している

strsafe.h のインクルードでなぜこんなことを..？
strsafe.h はCの標準ライブラリの代わりに作られた安全性の高い文字列操作関数が定義されている
→ VC++2005でより安全性が高い関数が作られた (string.hで_sが関数名の最後についている関数がそれ)
→ strsafe.h の関数には警告が出るようになった (その番号が 4996)

今書くならstrsafe.hを使わないほうが良い
*/
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 ) 

#pragma comment(lib, "d3d9.lib")


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
//ビデオチップ(GPU)の機能を調べたり、他のインターフェースのインスタンスを生成する(一番先に作る)
LPDIRECT3D9             g_pD3D = NULL; // Used to create the D3DDevice
//実際に描画処理をする機能のほとんどを持っている
//(Windowの表示対象と結びついているため、インターフェース作成時はウィンドウハンドルの横幅、高さ、色深度を指定する必要がある)
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device




//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
	// Create the D3D object, which is needed to create the D3DDevice.
	//Direct3DCreate9: IDirect3D9インターフェースのインスタンスを作成する
	//D3D_SDK_VERSION: バージョンチェックに使用. つねに D3D_SDK_VERSIONを指定する
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	//描画対象の属性設定
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;		//TRUE: ウィンドウ内に描画する, FALSE: フルスクリーン表示にする
	//スワップ後のバックバッファをどうするか? 
	//D3DSWAPEFFECT_DISCARD: ディスクドライバが自動判断(破棄される) → これ以外を指定することはまずない
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	//バックバッファーの色深度(最大表示色数)
	//D3DFMT_UNKNOWN: 現在のデスクトップ設定に従う
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	//CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, JWND hFocusWindow, DWORD BehaviourFlags, D3DPRESET_PARAMETERS *pPresentationParametetrs, IDirect3DDevice9** ppReturnedDeviceIntreface)
	//Adapter: 使用するディスプレイアダプタの指定(GPU/3Dビデオチップ)
	//D3DADAPTER_DEFAULT: 最優先のアダプタを使用する
	//複数のアダプタを切り替えたい場合は GetAdapterCountでアダプタの数を調べること
	//DeviceType: 処理を行うデバイスのタイプ(D3DDEVTYPE_HAL: 3Dビデオチップ, D3DDEVTYPE_REF: ソフトウェアだけで処理)
	//hFocusWindow: 描画対象のウィンドウハンドル
	//BehaviourFlags:　頂点演算をどう行うか. D3DCREATE_SOFTWARE_VERTEXPROCESSING か D3DCREATE_HARDWARE_VERTEXPROCESSINGを指定する
	//D3DCREATE_HARDWARE_VERTEXPROCESSING: GPUのハードウェアT&Lと呼ばれる頂点計算用回路(固定頂点機能パイプライン)を使用する
	//HARDWAREにしてGPU固定頂点機能パイプラインを使用したほうがパフォーマンスは上がる(対応しているかがわからない)
	//pPresentationParametetrs: 描画対象の属性ポインタ
	//ppReturnedDeviceIntreface: 作成されたIDirect3DDevice9のポインタのポインタ
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice)))
		/*if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev ) ) )
   if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev ) ) )
   if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev ) ) )
   if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev ) ) )
   という形見つかるまでやる方法もある*/
	{
		if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&d3dpp, &g_pd3dDevice)))
		{
			return E_FAIL;
		}
	}

	// Device state would normally be set here

	return S_OK;
}

//フルスクリーンモード
HRESULT InitD3D_Full(HWND hWnd)
{
	// Create the D3D object, which is needed to create the D3DDevice.
	//Direct3DCreate9: IDirect3D9インターフェースのインスタンスを作成する
	//D3D_SDK_VERSION: バージョンチェックに使用. つねに D3D_SDK_VERSIONを指定する
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	//フルスクリーンにはバックバッファーの幅と高さが、BackBufferFormatに色深度の指定が必要

	//バックバッファーの幅と高さを現在のデスクトップの設定に合わせたい → GetAdapterDisplayMode関数を用いる
	D3DDISPLAYMODE displayMode;
	ZeroMemory(&displayMode, sizeof(D3DDISPLAYMODE));
	/*
	GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode);
	Adapter: アダプタの指定
	pMode: 情報を返すポインタ
	*/
	g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);

	//描画対象の属性設定
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = FALSE;		//TRUE: ウィンドウ内に描画する, FALSE: フルスクリーン表示にする
	//スワップ後のバックバッファをどうするか? 
	//D3DSWAPEFFECT_DISCARD: ディスクドライバが自動判断(破棄される) → これ以外を指定することはまずない
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	//バックバッファの幅と高さ
	d3dpp.BackBufferHeight = displayMode.Height;
	d3dpp.BackBufferWidth = displayMode.Width;

	//バックバッファーの色深度(最大表示色数)
	d3dpp.BackBufferFormat = displayMode.Format;
	//リフレッシュレートの指定
	d3dpp.FullScreen_RefreshRateInHz = displayMode.RefreshRate;

	//CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, JWND hFocusWindow, DWORD BehaviourFlags, D3DPRESET_PARAMETERS *pPresentationParametetrs, IDirect3DDevice9** ppReturnedDeviceIntreface)
	//Adapter: 使用するディスプレイアダプタの指定(GPU/3Dビデオチップ)
	//D3DADAPTER_DEFAULT: 最優先のアダプタを使用する
	//複数のアダプタを切り替えたい場合は GetAdapterCountでアダプタの数を調べること
	//DeviceType: 処理を行うデバイスのタイプ(D3DDEVTYPE_HAL: 3Dビデオチップ, D3DDEVTYPE_REF: ソフトウェアだけで処理)
	//hFocusWindow: 描画対象のウィンドウハンドル
	//BehaviourFlags:　頂点演算をどう行うか. D3DCREATE_SOFTWARE_VERTEXPROCESSING か D3DCREATE_HARDWARE_VERTEXPROCESSINGを指定する
	//D3DCREATE_HARDWARE_VERTEXPROCESSING: GPUのハードウェアT&Lと呼ばれる頂点計算用回路(固定頂点機能パイプライン)を使用する
	//HARDWAREにしてGPU固定頂点機能パイプラインを使用したほうがパフォーマンスは上がる(対応しているかがわからない)
	//pPresentationParametetrs: 描画対象の属性ポインタ
	//ppReturnedDeviceIntreface: 作成されたIDirect3DDevice9のポインタのポインタ
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice)))
		/*if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev ) ) )
   if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev ) ) )
   if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev ) ) )
   if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev ) ) )
   という形見つかるまでやる方法もある*/
	{
		if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&d3dpp, &g_pd3dDevice)))
		{
			return E_FAIL;
		}
	}

	// Device state would normally be set here

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
	if (g_pd3dDevice != NULL)
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)
		g_pD3D->Release();
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
VOID Render()
{
	if (NULL == g_pd3dDevice)
		return;

	// Clear the backbuffer to a blue color
	//バックバッファの内容を消去(画面の塗りつぶし)
	/*
	HRESULT Clear(
		DWORD Count ,		消去する矩形の要素数(pRectsの数). 全体消去は0
		CONST D3DRECT* pRects ,	消去する矩形の配列. 全体消去はNULL
		DWORD Flags , 消去対象指定フラグ
		D3DCLEAR_TARGET: レンダリングターゲットを消去
		D3DCLEAR_ZBUFFER: 深度バッファを消去する
		D3DCLEAR_STENCIL: ステンシルバッファを消去する
		複数指定したい場合はビット演算で指定する D3DCLEAR_TARGET | D3DCLEAR_ZBUFFERのように
		D3DCOLOR Color ,	レンダリングターゲットを消去するときに使用する色
		レンダリングターゲット: バックバッファのピクセルのこと
		float Z ,		深度バッファを消去するときに使用する値(0 ～ 1)
		DWORD Stencil		ステンシルバッファを消去するときに使用する値

		D3DCOLOR_ARGB: arbgを指定
		D3DCOLOR_RGBA: rgbaを指定
		D3DCOLOR_XRGB: rgbを0 ～ 255で指定. アルファはつねに255
		D3DCOLOR_COLORVALUE; rgbaを0 ～ 1で指定
		アルファは透明度. 0は完全透明. 255は不透明
	);
	*/
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	// Begin the scene
	//描画開始通知
	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		// Rendering of scene objects can happen here
		//描画処理

		// End the scene
		//描画終了通知
		g_pd3dDevice->EndScene();
	}

	// Present the backbuffer contents to the display
	//バックバッファをスワップ
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
//メッセージプロシージャ
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		//デバイスの解放(DirectXのグローバルインスタンスを解放する)
		Cleanup();
		//WM_QUITをウィンドウプロシージャに送る
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
		Render();
		//ValidateRect: 描画が終わったことをWindowsに伝える
		ValidateRect(hWnd, NULL);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// Register the window class
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
					  GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
					  "D3D Tutorial", NULL };
	RegisterClassEx(&wc);

	// Create the application's window
	HWND hWnd = CreateWindow("D3D Tutorial", "D3D Tutorial 01: CreateDevice",
		WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
		NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	// Init3Dの返り値がS_OKの時trueになる
	if (SUCCEEDED(InitD3D(hWnd)))
	{
		// Show the window
		ShowWindow(hWnd, SW_SHOWDEFAULT);
		//hwndが空ではない場合、ウィンドウへ WM_PAINT メッセージを送信する(ウィンドウのクライアント領域の更新)
		UpdateWindow(hWnd);

		// Enter the message loop
		MSG msg;
		//GetMessage: メッセージキュー内のメッセージを取り出し、msgに格納する. WM_QUIT メッセージを取得した場合、0が返り値となる
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	UnregisterClass("D3D Tutorial", wc.hInstance);
	return 0;
}



