//-----------------------------------------------------------------------------
// File: Meshes.cpp
//
// Desc: For advanced geometry, most apps will prefer to load pre-authored
//       meshes from a file. Fortunately, when using meshes, D3DX does most of
//       the work for this, parsing a geometry file and creating vertx buffers
//       (and index buffers) for us. This tutorial shows how to use a D3DXMESH
//       object, including loading it from a file and rendering it. One thing
//       D3DX does not handle for us is the materials and textures for a mesh,
//       so note that we have to handle those manually.
//
//       Note: one advanced (but nice) feature that we don't show here is that
//       when cloning a mesh we can specify the FVF. So, regardless of how the
//       mesh was authored, we can add/remove normals, add more texture
//       coordinate sets (for multi-texturing), etc.
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




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9         g_pD3D = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9   g_pd3dDevice = NULL; // Our rendering device


//tiger.xから読み取ったmeshデータはこっち
//meshデータはID3DXMeshインターフェースのインスタンスに格納する
//LPD3DXMESH は ID3DXMesh のポインタ (typedef struct ID3DXMesh *LPD3DXMESH;)
LPD3DXMESH          g_pMesh = NULL; // Our mesh object in sysmem
//materialデータはこっち (D3DMATERIAL9に格納する)
D3DMATERIAL9* g_pMeshMaterials = NULL; // Materials for our mesh
//tiger.bmpから読み取ったテクスチャデータはこっち
//テクスチャはIDirect3DTexture9のインスタンスに格納. そのポインタがLPDIRECT3DTEXTURE9
LPDIRECT3DTEXTURE9* g_pMeshTextures = NULL; // Textures for our mesh
//マテリアルの数を指定
DWORD               g_dwNumMaterials = 0L;   // Number of mesh materials

//D3DMATERIAL9とLPDIRECT3DTEXTURE9は1つのモデルに複数設定されることがあるため、
//ポインタにして配列で扱えるようにしている

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
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    // Create the D3DDevice
    if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp, &g_pd3dDevice)))
    {
        return E_FAIL;
    }

    // Turn on the zbuffer
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

    // Turn on ambient lighting 
    g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0xffffffff);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Load the mesh and build the material and texture arrays
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
    //マテリアルデータ取得用変数
    //typedef interface ID3DXBuffer *LPD3DXBUFFER;
    LPD3DXBUFFER pD3DXMtrlBuffer;

    // Load the mesh from the specified file
    if (FAILED(D3DXLoadMeshFromX(L"Tiger.x", D3DXMESH_SYSTEMMEM,
        g_pd3dDevice, NULL,
        &pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
        &g_pMesh)))
    {
        // If model is not in current folder, try parent folder
        //失敗したら、1つ上の階層に置いてないか確認
        if (FAILED(D3DXLoadMeshFromX(L"..\\Tiger.x", D3DXMESH_SYSTEMMEM,
            g_pd3dDevice, NULL,
            &pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
            &g_pMesh)))
        {
            MessageBox(NULL, L"Could not find tiger.x", L"Meshes.exe", MB_OK);
            return E_FAIL;
        }
    }

    //D3DXLoadMeshFromXで読み込んだマテリアルデータはID3DXBuffer型インターフェースのインスタンスに格納される
    //→ Meshを読み込むときに使うデータ記憶用のインターフェースなので使えない
    //GetBufferPointerでデータ編集用アドレスを取得 → D3DXMATERIAL型のポインタに代入
    /*
    typedef struct _D3DXMATERIAL
    {
        D3DMATERIAL9  MatD3D;       //マテリアル(色情報)
        LPSTR         pTextureFilename;     //テクスチャのファイル名
    } D3DXMATERIAL;
    */
    D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
    //D3DXMATERIALの内容をコピーするためにD3DXMATERIALと同じ要素数のD3DXMATERIALの配列を動的確保
    g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
    if (g_pMeshMaterials == NULL)
        return E_OUTOFMEMORY;
    //D3DXMATERIALの内容をコピーするためにD3DXMATERIALと同じ要素数のIDirect3DTexture9(LPDIRECT3DTEXTURE9)の配列を動的確保
    g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
    if (g_pMeshTextures == NULL)
        return E_OUTOFMEMORY;

    //マテリアルのコピーとテクスチャの読み込み
    for (DWORD i = 0; i < g_dwNumMaterials; i++)
    {
        // Copy the material
        //マテリアルのコピー
        g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

        // Set the ambient color for the material (D3DX does not do this)
        //XファイルにはAmbientがないので、マテリアルtのDiffuseを使用
        g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

        //テクスチャの参照にNULLを代入(読み込み失敗時、マテリアルでテクスチャの指定がなかったときにNULL)
        g_pMeshTextures[i] = NULL;
        if (d3dxMaterials[i].pTextureFilename != NULL &&        //ファイル名が指定されてない(ファイル名のポインタがNULLではない)
            //lstrlenA関数は文字列の長さをバイト数で返す
            lstrlenA(d3dxMaterials[i].pTextureFilename) > 0)        //ファイル名が1文字以上指定されている(空文字はない)
        {
            // Create the texture
            /*
            HRESULT D3DXCreateTextureFromFile(
              _In_  LPDIRECT3DDEVICE9  pDevice,     //デバイスのポインタ
              _In_  LPCTSTR            pSrcFile,        //テクスチャファイル名
              _Out_ LPDIRECT3DTEXTURE9 *ppTexture       //読み込んだテクスチャを返すポインタ
            );
            */
            if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice,
                d3dxMaterials[i].pTextureFilename,
                &g_pMeshTextures[i])))
            {
                // If texture is not in current folder, try parent folder
                //テクスチャの読み込みが失敗したときに、一つ上のフォルダ構造で読み込みを試みる
                const CHAR* strPrefix = "..\\";
                //ファイルパス保存用の配列を用意
                CHAR strTexture[MAX_PATH];
                //NULL終端文字列をコピー. strTextureにstrPrefixの文字列をコピー(つまり、strTexture = "..\\")
                strcpy_s(strTexture, MAX_PATH, strPrefix);
                //文字列を連結. strTextureにd3dxMaterials[i].pTextureFilenameを連結する
                strcat_s(strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename);
                // If texture is not in current folder, try parent folder
                if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice,
                    strTexture,
                    &g_pMeshTextures[i])))
                {
                    MessageBox(NULL, L"Could not find texture map", L"Meshes.exe", MB_OK);
                }
            }
        }
    }

    // Done with the material buffer
    //一時的にマテリアルの情報を入れていたバッファは不要なので解放
    pD3DXMtrlBuffer->Release();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
//描画終了時にメッシュやマテリアルは開放する必要がある
//newで確保したらdeleteで解放（配列はdelete[]）
VOID Cleanup()
{
    if (g_pMeshMaterials != NULL)
        delete[] g_pMeshMaterials;

    if (g_pMeshTextures)        //テクスチャのポインタ配列のため、配列だけ削除してもテクスチャ本体は開放されない
    {
        for (DWORD i = 0; i < g_dwNumMaterials; i++)
        {
            if (g_pMeshTextures[i])
                //Texture本体を解放する
                g_pMeshTextures[i]->Release();
        }
        //本体を解放したのでg_pMeshTexturesも解放する
        delete[] g_pMeshTextures;
    }
    if (g_pMesh != NULL)
        g_pMesh->Release();

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
    // Set up world matrix
    D3DXMATRIXA16 matWorld;
    D3DXMatrixRotationY(&matWorld, timeGetTime() / 1000.0f);
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. Here, we set the
    // eye five units back along the z-axis and up three units, look at the 
    // origin, and define "up" to be in the y-direction.
    D3DXVECTOR3 vEyePt(0.0f, 3.0f, -5.0f);
    D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

    // For the projection matrix, we set up a perspective transform (which
    // transforms geometry from 3D view space to 2D viewport space, with
    // a perspective divide making objects smaller in the distance). To build
    // a perpsective transform, we need the field of view (1/4 pi is common),
    // the aspect ratio, and the near and far clipping planes (which define at
    // what distances geometry should be no longer be rendered).
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
VOID Render()
{
    // Clear the backbuffer and the zbuffer
    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

    // Begin the scene
    if (SUCCEEDED(g_pd3dDevice->BeginScene()))
    {
        // Setup the world, view, and projection matrices
        //変換行列の設定
        SetupMatrices();

        // Meshes are divided into subsets, one for each material. Render them in
        // a loop
        //描画
        for (DWORD i = 0; i < g_dwNumMaterials; i++)
        {
            // Set the material and texture for this subset
            //マテリアルの設定
            g_pd3dDevice->SetMaterial(&g_pMeshMaterials[i]);
            /*
            HRESULT SetTexture(
              DWORD                 Stage,      //サンプラ番号(プログラマブルシェーダーやテクスチャ合成時に使用)
              IDirect3DBaseTexture9 *pTexture       //設定するテクスチャのポインタ
            );
            */
            //テクスチャの設定
            g_pd3dDevice->SetTexture(0, g_pMeshTextures[i]);

            // Draw the mesh subset
            //Meshを描画
            /*
            ID3DXBaseMesh::DrawSubset関数
               HRESULT DrawSubset(
                  [in] DWORD AttribId		//サブセット番号. Mesh内ではMaterialごとにパーツ分けされている. 
                  //その管理番号 = サブセット番号
               );
            */
            g_pMesh->DrawSubset(i);
        }

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
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, INT)
{
    UNREFERENCED_PARAMETER(hInst);

    // Register the window class
    WNDCLASSEX wc =
    {
        sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
        GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
        L"D3D Tutorial", NULL
    };
    RegisterClassEx(&wc);

    // Create the application's window
    HWND hWnd = CreateWindow(L"D3D Tutorial", L"D3D Tutorial 06: Meshes",
        WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
        NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (SUCCEEDED(InitD3D(hWnd)))
    {
        // Create the scene geometry
        if (SUCCEEDED(InitGeometry()))
        {
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
                    Render();
            }
        }
    }

    UnregisterClass(L"D3D Tutorial", wc.hInstance);
    return 0;
}



