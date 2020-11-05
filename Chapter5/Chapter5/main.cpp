#include "main.h"
#include  "Chapter5.h"

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	if (SUCCEEDED(InitD3DWindow(_T("Chapter5 Window"), 640, 480)))
	{
		//メッセージループ
		MSG msg = { 0 };
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	UnregisterClass(_T("Chapter5 Window"), GetModuleHandle(NULL));
	return 0;
}