#pragma once

#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#include <tchar.h>
#include <dinput.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

struct Model
{
	//メッシュ
	LPD3DXMESH pMesh;
	//マテリアルの配列
	D3DMATERIAL9* pMaterial;
	//テクスチャの配列
	LPDIRECT3DTEXTURE9* pTexture;
	//マテリアルの数
	DWORD numMaterials;

	//データが入っているかのフラグ
	bool used;
};

//グローバル変数
//他のソースコードでも使用できるようにexternをつける
extern LPDIRECT3D9 g_pD3D;
extern LPDIRECT3DDEVICE9 g_pd3DDeivece;

extern float g_aspect;

extern Model g_models[];

//関数プロトタイプ宣言
HRESULT InitD3DWindow(LPCTSTR winTitle, int w, int h);
int LoadModel(LPCTSTR fileName);
void RenderModel(int idx);
const char* GetKeyState();
void setTimer(int idx, DWORD time);
BOOL isTimerGoal(int idx);
//経過時間を取得する
DWORD getPassedTime(int idx);