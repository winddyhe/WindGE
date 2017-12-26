#include "Win32Window.h"

using namespace WindGE;

Win32Window* g_window1 = 0;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return g_window1->msg_proc(hwnd, msg, wParam, lParam);
}

Win32Window::Win32Window(HINSTANCE hInst) :
	__inst(hInst),
	__hwnd(0),
	__is_paused(false),
	__minimized(false),
	__maximized(false),
	__resizing(false),
	__title_name(L"App"),
	__width(1000),
	__height(620),
	__frame_count(0),
	__app(nullptr)
{
	g_window1 = this;
}


Win32Window::~Win32Window()
{
}

bool Win32Window::init(Application* app, std::wstring titleName) 
{
	__app = app;
	__title_name = titleName;

	if (!__app)					return false;
	if (!_init_main_window())	return false;
	if (!_init_app())			return false;

	return true;
}

void Win32Window::resize()
{
	__app->resize(__width, __height);
}

int Win32Window::run() 
{
	MSG msg = { 0 };

	__timer.reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			__timer.tick();
			if (!__is_paused)
			{
				_calculate_frame_stats();
				
				__app->update(__timer.delta_time());
				__app->draw();
			}
			else
			{
				Sleep(100);
			}
		}
	}
	return (int)msg.wParam;
}

LRESULT Win32Window::msg_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			__is_paused = true;
			__timer.stop();
		}
		else
		{
			__is_paused = false;
			__timer.start();
		}
		return 0;

	case WM_SIZE:
		__width = LOWORD(lParam);
		__height = HIWORD(lParam);
		if (__app)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				__is_paused = true;
				__minimized = true;
				__maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				__is_paused = false;
				__minimized = false;
				__maximized = true;
				resize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (__minimized)
				{
					__is_paused = false;
					__maximized = false;
					resize();
				}
				else if (__maximized)
				{
					__is_paused = false;
					__maximized = false;
					resize();
				}
				else if (__resizing)
				{
					//todo
				}
				else
				{
					resize();
				}
			}
		}
		return 0;

	case WM_ENTERSIZEMOVE:
		__is_paused = true;
		__resizing = true;
		__timer.stop();
		return 0;

	case WM_EXITSIZEMOVE:
		__is_paused = false;
		__resizing = false;
		__timer.start();
		resize();
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		on_mouse_down(wParam, GET_X_LPARAM(lParam), GET_X_LPARAM(lParam));
		return 0;

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		on_mouse_up(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEMOVE:
		on_mouse_move(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool Win32Window::_init_main_window()
{
	WNDCLASSEX wc;

	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= MainWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= __inst;
	wc.hIcon			= LoadIcon(0, IDI_APPLICATION);
	wc.hCursor			= LoadCursor(0, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= L"AppWndClassName";
	wc.hIconSm			= LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT R = { 0, 0, __width, __height };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	__hwnd = CreateWindowEx(0,
		L"AppWndClassName",
		__title_name.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		0,
		0,
		__inst,
		0);

	if (!__hwnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(__hwnd, SW_SHOW);
	UpdateWindow(__hwnd);
	SetWindowPos(__hwnd, NULL, 50, 50, __width, __height, NULL);

	return true;
}

bool Win32Window::_init_app()
{
	if (!__app->init(__inst, __hwnd, __width, __height)) return false;

	resize();

	return true;
}

void Win32Window::_calculate_frame_stats()
{
	static float timeElapsed = 0.0f;

	__frame_count++;

	if ((__timer.total_time() - timeElapsed) >= 1.0f)
	{
		std::wostringstream outs;
		outs.precision(6);
		outs.imbue(std::locale("CHS"));

		float fps = (float)__frame_count;
		float mspf = 1000.0f / fps;

		outs << __title_name << L"   FPS: " << fps << L", " << L"Frame Time: " << mspf << L" (ms)";

		SetWindowText(__hwnd, outs.str().c_str());

		__frame_count = 0;
		timeElapsed += 1.0f;
	}
}