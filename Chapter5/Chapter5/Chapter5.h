#pragma once

#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#include <tchar.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

//グローバル変数
//他のソースコードでも使用できるようにexternをつける
extern LPDIRECT3D9 g_pD3D;
extern LPDIRECT3DDEVICE9 g_pd3DDeivece;

extern float g_aspect;

//関数プロトタイプ宣言
HRESULT InitD3DWindow(LPCTSTR winTitle, int w, int h);
