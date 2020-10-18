// Chapter3.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "Chapter3.h"

#include <windows.h>
#include <tchar.h>

LRESULT WINAPI MsgProc(HWND, UINT, WPARAM, LPARAM);

/*
WinMain：アプリケーションのエントリ ポイント
WINAPI: 呼び出し規約
呼び出し規約とは？→ 関数呼び出しの際の引数の渡し方を決めるc++のルール
「_cdecl」「_stdcall」「_fastcall」があり、Windowsから呼ばれるのは「_stdcall」を付ける必要がある
#defineプリプロセッサでわかりやすくWINAPIとつけられている

hInst: 現在のインスタンスのハンドル(識別番号)
オブジェクトの識別番号のこと。同じアプリケーションを開いた際に、メモリ上に同じプログラムが存在することになるが、
それ区別するために使用する
hpInst: 以前のインスタンスのハンドル
昔のWindowsのために残されているオブジェクトの識別番号. 普通NULLが入る
lpCmd: コマンドライン
LPSTR→ 32bitアドレス用ポインタ(LP: long pointer. 昔使用されていた short pointerと区別している)
コマンドライン引数のアドレスが格納される (例えば explorer /e としたときの/eが調べられうy)
nCmdShow: 表示状態
ウィンドウの状態を表す変数
*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hpInst, LPSTR lpCmd, int nCmd)
{
	//MessageBox(NULL, _T("Hello"), _T("Test"), MB_OK);
	//ウィンドウクラスの作成
	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX),		//構造体のサイズ
		CS_CLASSDC,		//ウィンドウクラスのスタイル
		MsgProc,		//ウィンドウプロシージャルのアドレス
		0L,
		0L,
		hInst,
		NULL,
		NULL,
		NULL,
		NULL,
		_T("My Window"),
		NULL
	};
	//Windowsに登録
	RegisterClassEx(&wc);

	//ウィンドウの作成
	//HWND: ウィンドウハンドル -> ウィンドウを識別する番号. 作成したウィンドウを他で制御するときに使う
	HWND hwnd = CreateWindow(_T("My Window"), _T("ウィンドウ名"), WS_OVERLAPPEDWINDOW/* | WS_SIZEBOX*/, 100, 100, 300, 300, NULL, NULL, hInst, NULL);
	ShowWindow(hwnd, SW_SHOWDEFAULT);

	//メッセージループ
	MSG msg;
	//渡された変数を0で初期化
	ZeroMemory(&msg, sizeof(MSG));
	//WM_QUITが来たら抜ける
	while (msg.message != WM_QUIT)
	{
		//PeekMessage: windowからメッセージが転送されるのでそれを受信. msg変数に保存
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			//必須じゃない. 
			//キーボードメッセージから文字メッセージを生成するために使用。
			//TranslateMessageがWM_KEYDOWNを検出すると、文字メッセージであるWM_CHARがメッセージキューに格納され、 次にGetMessageを呼び出した際に取得されることになる

			//通常、アプリケーションがキーを押すとWM_KEYDOWNというキーボードメッセージが送られ、 たとえばAキーならwParamにはAキーの仮想キーコードである0x41が格納される
			//しかし、仮想キーコードだけは、押したAが大文字のAなのか小文字のaなのか分かないため、
			//このような場合は文字コードをwParamとして持つ文字メッセージが必要になる
			TranslateMessage(&msg);
			//ウィンドウプロシージャ(今回はBUTTONのプロシージャ)にメッセージを転送する
			DispatchMessage(&msg);
		}
	}

	//Windowsに登録したウィンドウクラスの解除
	UnregisterClass(_T("My Window"), hInst);
	return 0;
}

//ウィンドウプロシージャ. 関数名は自由だが、引数と返り値は指定あり
//LRESULT: 結果を返す. 実態はlong int
//ウィンドウプロシージャはWindowsから呼び出されるので呼び出し規約をWINAPIとする
//CALLBACKでもよいが、WINAPIと全く同じ
//引数ははDispatchMessageで転送した内容が入る
LRESULT WINAPI MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//msgにはイベントを表す定数が入っている
	switch (msg)
	{
	case WM_DESTROY:		//ウィンドウを閉じるとき
		//特に問題がないことを伝えるために0
		PostQuitMessage(0);		//WM_QUITメッセージを送る → Windowsを通してメッセージループに渡り、WinMain関数のメッセージループを抜ける
		return 0;
	}

	//自作のウィンドウプロシージャルで処理しないイベントメッセージはDefWindowProcに任せれば、ウィンドウサイズ変更などをしてくれる
	return DefWindowProc(hwnd, msg, wParam, lParam);
}