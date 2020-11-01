//-----------------------------------------------------------------------------
// File: Lights.cpp
//
// Desc: Rendering 3D geometry is much more interesting when dynamic lighting
//       is added to the scene. To use lighting in D3D, you must create one or
//       lights, setup a material, and make sure your geometry contains surface
//       normals. Lights may have a position, a color, and be of a certain type
//       such as directional (light comes from one direction), point (light
//       comes from a specific x,y,z coordinate and radiates in all directions)
//       or spotlight. Materials describe the surface of your geometry,
//       specifically, how it gets lit (diffuse color, ambient color, etc.).
//       Surface normals are part of a vertex, and are needed for the D3D's
//       internal lighting calculations.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#include <tchar.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 ) 


const int SURFACE_NUM = 10;


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9             g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // Buffer to hold vertices

// A structure for our custom vertex type. We added a normal, and omitted the
// color (which is provided by the material)
//1つの頂点データを表す構造体
//CUSTOMVERTEX: D3DXVECTOR3のメンバを2つ持った構造体
struct CUSTOMVERTEX
{
	D3DXVECTOR3 position; // The 3D position for the vertex
	D3DXVECTOR3 normal;   // The surface normal for the vertex
	DWORD diffColor;
};

// Our custom FVF, which describes our custom vertex structure
//D3DFVF_CUSTOMVERTEX: カスタムで作成したもの. D3DFVF_XYZとD3DFVF_NORMALをor演算子でつなげたもの
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE)




//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
	// Create the D3D object.
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	// Set up the structure used to create the D3DDevice. Since we are now
	// using more complex geometry, we will create a device with a zbuffer.
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	//zバッファを有効(深度ステンシルバッファ作成指示). これがないとzテストが有効にならない
	d3dpp.EnableAutoDepthStencil = TRUE;
	//深度バッファの形式指定(D3DFMT_D16: 1要素あたり16bitのもの)
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// Create the D3DDevice
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice)))
	{
		return E_FAIL;
	}

	// Turn off culling
	//SetRenderState: レンダリングの設定を行う. 第一引数: 何を設定するか, 第二引数: 設定値の指定
	//D3DRS_CULLMODE: 後ろ向きの三角形をカリングする方法を指定. 
	//D3DCULL_NONE: カリングなし(裏向きのポリゴンの描画を省略しない)
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// Turn on the zbuffer
	//zバッファを有効にするか

	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Creates the scene geometry
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
	// Create the vertex buffer.
	//頂点バッファはIDirect3DVertexBuffer9インターフェースのインスタンス。頂点データを記録するためのメモリ領域確保
	//CreateVertexBuffer: 指定したサイズの頂点が格納できるインスタンスがメモリ上に作成される
	//UNIT Length: 頂点バッファの大きさ(サイズ)
	//DWORD Usage: 使用方法. 通常0. (CPUから参照許可するとかのときに指定したりする)
	//DWORD FVF: 頂点データ形式(頂点フォーマット). 頂点に含まれる情報を指定する
	//D3DPOOL Pool: バッファが作成される場所を指定するか.指定しない場合は
	//IDirect3DVertexBuffer9** ppVertexBuffer: バッファのアドレスが返却されるポインタ
	//HANDLE* pHandle: 現在は使用されていない値. NULLを入れる
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(SURFACE_NUM * 6 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		// D3DPOOL_DEFAULT: リソースを、リソースに対して要求された使用方法に最も適したメモリプールに置く
		D3DPOOL_DEFAULT, &g_pVB, NULL)))
	{
		return E_FAIL;
	}

	// Fill the vertex buffer. We are algorithmically generating a cylinder
	// here, including the normals, which are used for lighting.
	CUSTOMVERTEX* pVertices;
	//頂点バッファが作成できたら、データを記録(編集時)するため頂点バッファをロック
	//ロックするとGPUからのアクセスを止める
	//ロック解除までGPUは描画を待つので長時間ロックしないようにする
	//引数↓
	//UINT OffsetToLock: ロックの開始位置. 先頭の場合は0から
	//UNIT SizeToLock: ロック範囲. 全体の場合は0
	//VOID **ppbData: 編集用のアドレスを記録するポインタ→ CUSTOMVERTEX型のポインタのポインタを指定する
	//DWORD Flags: ロック処理の設定
	//通常 D3DLOCK_DISCARD を指定 → 書き込み用に新しい領域を確保し、古い領域を破棄する(ロック時の遅延が最小になる)
	if (FAILED(g_pVB->Lock(0, 0, (void**)&pVertices, 0)))
		//void型のポインタとは? → 型指定のないポインタという意味になる
		//⇒ void型のポインタには全型のポインタの内容を代入できる
		//頂点バッファとして様々な型データが使えるのはそれが理由
		//注意: コンパイラのチェック対象となるのでポインタの向き先にどういうのが入っているのかを知らないといけない
		return E_FAIL;
	for (DWORD i = 0; i < SURFACE_NUM; i++)
	{
		FLOAT theta = (2 * D3DX_PI * i) / SURFACE_NUM;
		FLOAT thetaPlus = (2 * D3DX_PI * (i+1)) / SURFACE_NUM;
		pVertices[6 * i + 0].position = D3DXVECTOR3(sinf(theta), -1.0f, cosf(theta));
		pVertices[6 * i + 0].normal = D3DXVECTOR3(sinf(theta), 0.0f, cosf(theta));
		//DWORD は ARGB
		pVertices[6 * i + 1].position = D3DXVECTOR3(sinf(theta), 1.0f, cosf(theta));
		pVertices[6 * i + 1].normal = D3DXVECTOR3(sinf(theta), 0.0f, cosf(theta));
		pVertices[6 * i + 2].position = D3DXVECTOR3(sinf(thetaPlus), 1.0f, cosf(thetaPlus));
		pVertices[6 * i + 2].normal = D3DXVECTOR3(sinf(thetaPlus), 0.0f, cosf(thetaPlus));
		pVertices[6 * i + 0].diffColor = 0xffff0000;
		pVertices[6 * i + 1].diffColor = 0xffff0000;
		pVertices[6 * i + 2].diffColor = 0xffff0000;

		pVertices[6 * i + 3].position = D3DXVECTOR3(sinf(theta), -1.0f, cosf(theta));
		pVertices[6 * i + 3].normal = D3DXVECTOR3(sinf(theta), 0.0f, cosf(theta));
		pVertices[6 * i + 4].position = D3DXVECTOR3(sinf(thetaPlus), -1.0f, cosf(thetaPlus));
		pVertices[6 * i + 4].normal = D3DXVECTOR3(sinf(thetaPlus), 0.0f, cosf(thetaPlus));
		pVertices[6 * i + 5].position = D3DXVECTOR3(sinf(thetaPlus), 1.0f, cosf(thetaPlus));
		pVertices[6 * i + 5].normal = D3DXVECTOR3(sinf(thetaPlus), 0.0f, cosf(thetaPlus));
		pVertices[6 * i + 3].diffColor = 0xffff0000;
		pVertices[6 * i + 4].diffColor = 0xffff0000;
		pVertices[6 * i + 5].diffColor = 0xffff0000;
	}
	g_pVB->Unlock();

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
	if (g_pVB != NULL)
		g_pVB->Release();

	if (g_pd3dDevice != NULL)
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)
		g_pD3D->Release();
}



//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Sets up the world, view, and projection transform matrices.
//-----------------------------------------------------------------------------
VOID SetupMatrices()
{
	//行列を記録する構造体
	/*
	いずれも16個のfloatを持つ
	D3DMATRIX: メンバを持つだけの構造体
	D3DXMATRIX: D3DMATRIXにc++演算子がオーバーロードされて使いやすくなったもの
	D3DXMATRIXA16: 16byteを整列させることでD3DXMATRIXよりメモリ処理の効率を上げたもの
	*/

	// Set up world matrix
	//ワールド行列の作成
	D3DXMATRIXA16 matWorld;
	//単位行列に初期化
	D3DXMatrixIdentity(&matWorld);
	//x軸を中心に回転する変換行列
	D3DXMatrixRotationX(&matWorld, timeGetTime() / 500.0f);
	//D3DXMatrixRotationX(&matWorld, timeGetTime() / 500.0f);
	//変換行列の設定
	/*
	D3DTS_WORLD: ワール変換行列
	D3DTS_VIEW: ビュー変換行列
	D3DTS_PROJECTION: プロジェクション変換行列(射影行列)
	*/
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

	// Set up our view matrix. A view matrix can be defined given an eye point,
	// a point to lookat, and a direction for which way is up. Here, we set the
	// eye five units back along the z-axis and up three units, look at the
	// origin, and define "up" to be in the y-direction.
	D3DXVECTOR3 vEyePt(0.0f, 5.0f, 10.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIXA16 matView;
	/*
	D3DXMATRIX* D3DXMatrixLookAtLH(
	  _Inout_       D3DXMATRIX  *pOut,		//結果のポインタ
	  _In_    const D3DXVECTOR3 *pEye,		//カメラの位置
	  _In_    const D3DXVECTOR3 *pAt,		//注視点
	  _In_    const D3DXVECTOR3 *pUp		//上方向を表すベクトル
	);
	*/
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	// For the projection matrix, we set up a perspective transform (which
	// transforms geometry from 3D view space to 2D viewport space, with
	// a perspective divide making objects smaller in the distance). To build
	// a perpsective transform, we need the field of view (1/4 pi is common),
	// the aspect ratio, and the near and far clipping planes (which define at
	// what distances geometry should be no longer be rendered).
	D3DXMATRIXA16 matProj;
	//射影変換行列
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}




//-----------------------------------------------------------------------------
// Name: SetupLights()
// Desc: Sets up the lights and materials for the scene.
//-----------------------------------------------------------------------------
VOID SetupLights()
{
	// Set up a material. The material here just has the diffuse and ambient
	// colors set to yellow. Note that only one material can be used at a time.
	//マテリアルの設定
	/*
	typedef struct D3DMATERIAL9 {
	  D3DCOLORVALUE Diffuse; →	黄色
	  D3DCOLORVALUE Ambient; → 黄色
	  D3DCOLORVALUE Specular;
	  D3DCOLORVALUE Emissive;
	  float         Power;
	} D3DMATERIAL9, *LPD3DMATERIAL9;
	*/
	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
	mtrl.Diffuse.r  = 0.0f;
	mtrl.Ambient.r = 0.0f;
	mtrl.Diffuse.g = 1.0f;
	mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b =  0.0f;
	mtrl.Ambient.b = 0.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	//mtrl.Power = 0.01f;
	//全ポリゴンに設定される
	//ポリゴンごとに色を変える場合は diffuse色が記録できる頂点バッファにする or ポリゴンを書くごとにMaterialを切り替える
	g_pd3dDevice->SetMaterial(&mtrl);

	// Set up a white, directional light, with an oscillating direction.
	// Note that many lights may be active at a time (but each one slows down
	// the rendering of our scene). However, here we are just using one. Also,
	// we need to set the D3DRS_LIGHTING renderstate to enable lighting
	//ライトの向き、色の設定
	D3DXVECTOR3 vecDir;
	D3DLIGHT9 light;
	/*
	typedef struct D3DLIGHT9 {
	  D3DLIGHTTYPE  Type;		//光源の種類
	  D3DCOLORVALUE Diffuse;
	  D3DCOLORVALUE Specular;
	  D3DCOLORVALUE Ambient;
	  D3DVECTOR     Position;
	  D3DVECTOR     Direction;
	  float         Range;		//有効範囲
		  //平行光源を選択した場合以下は無効
	  float         Falloff;	//スポットライトの内部と外部の正面減少(基本 1)
	  float         Attenuation0;		//減衰1
	  float         Attenuation1;		//減衰2
	  float         Attenuation2;		//減衰3
	  float         Theta;		//内部コーンの角度
	  float         Phi;		//外部コーンの角度
	} D3DLIGHT9, *LPD3DLIGHT;
	*/
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 1.0f;
	light.Diffuse.b = 1.0f;
	//timeGetTime(): Windows起動時からの経過時間(ミリ秒)
	/*vecDir = D3DXVECTOR3(cosf(timeGetTime() / 350.0f),
		1.0f,
		sinf(timeGetTime() / 350.0f));*/
	vecDir = D3DXVECTOR3(1, 0, 0);
		/*
	D3DXVec3Normalize(
		D3DXVECTOR3* pOut,		//正規後のベクトルのポインタ
		CONST D3DXVECTOR3* pV		//正規前のベクトルのポインタ
	)
	*/
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);
	light.Range = 1000.0f;
	//0番のライトの設定
	g_pd3dDevice->SetLight(0, &light);
	//0番のライトを有効
	g_pd3dDevice->LightEnable(0, TRUE);
	//ライトのライティングを行う(デフォルトTRUEになっているようなので、書かなくても描画される)
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

	// Finally, turn on some ambient light.
	//環境光のアンビエント色の設定
	//→ 影の部分の色
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0xff000000);
}

VOID Render_Detail()
{
	// Setup the world, view, and projection matrices
	SetupMatrices();

	//頂点バッファーをDirect3Dデバイスに渡す
	//SetStreamSource：頂点バッファのアドレスをデバイスに伝える
	// Render the vertex buffer contents
	//SetStreamSourceの引数↓
	//UINT StreamNumber：ストリームの番号. 通常0 (詳細はURL参考; http://marupeke296.com/DXG_No53_DrawUsingStream.html)
	//IDirect3DVertexBuffer9 *pStreamData: 頂点バッファのポインタ
	//UINT OffsetInBytes: データ本体までのオフセット
	//UINT Stride; 頂点1つ分のデータのバイト数(サイズ)
	g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
	//SetFVF: 頂点に含まれるデータを表す定数を伝える → 今回は位置と法線ですよと教えている
	g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	//DrawPrimitive: 描画
	//D3DPRIMITIVETYPE: 頂点のレンダリング方法
	//D3DPT_TRIANGLESTRIP: 連続した三角形としてレンダリング（トライアングルストリップという方法: ポリゴンの辺が共有されているときに、再利用してポリゴンを連続描画する方法）
	//UINT StartVertex; 描画を始める頂点の番号. 先頭は0
	//UINT PrimitiveCount: 描画する頂点の数
	g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, SURFACE_NUM * 2);

	D3DXMATRIXA16 matWorld, transWorld, rotateWorld;
	//設定済みのワールド変換行列 = 真ん中に置いた円柱の回転行列を取得する
	g_pd3dDevice->GetTransform(D3DTS_WORLD, &rotateWorld);

	D3DXMatrixTranslation(&transWorld, -3, 0, 2);
	matWorld = rotateWorld * transWorld;
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
	g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, SURFACE_NUM * 2);

	D3DXMatrixTranslation(&transWorld, 3, 0, 2);
	matWorld = rotateWorld * transWorld;
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
	g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, SURFACE_NUM * 2);

}


//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
VOID Render()
{
	// Clear the backbuffer and the zbuffer
	//D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER: バックバッファーとZバッファーを初期化している
	//0が手前、1が奥
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	// Begin the scene
	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		Render_Detail();

		// End the scene
		g_pd3dDevice->EndScene();
	}

	// Present the backbuffer contents to the display
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}


//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		return 0;
		//前回はWM_PAINTで描画していたが、今回していないのは？
		//→ ウィンドウの初期表示時、背面に隠れていたウィンドウが前に来たときに呼ばれるもののため常に描画するのには向いていない
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
					  _T("D3D Tutorial"), NULL };
	RegisterClassEx(&wc);

	// Create the application's window
	HWND hWnd = CreateWindow(_T("D3D Tutorial"), _T("D3D Tutorial 04: Lights"),
		WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
		NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (SUCCEEDED(InitD3D(hWnd)))
	{
		// Create the geometry
		//3Dモデルの作成
		if (SUCCEEDED(InitGeometry()))
		{
			// Setup the lights and materials
			SetupLights();

			// Show the window
			ShowWindow(hWnd, SW_SHOWDEFAULT);
			UpdateWindow(hWnd);

			// Enter the message loop
			MSG msg;
			ZeroMemory(&msg, sizeof(msg));
			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
					//windowsからメッセージプロシージャが来ないとき=PCの可能な限り早い速さで呼ばれる
					//elseはmsg.message = WM_DESTORYのときに、Render関数が呼ばれないようにするため(呼ぶとWM_DESTORYのときはデバイス解放後に呼ばれることになるためエラーとなる)
					Render();
			}
		}
	}

	UnregisterClass(_T("D3D Tutorial"), wc.hInstance);
	return 0;
}



