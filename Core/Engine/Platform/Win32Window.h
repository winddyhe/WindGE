#pragma once
#ifndef __WIND32WINDOW_H__
#define __WIND32WINDOW_H__

#include "../Core/Config.h"
#include "../Core/Timer.h"
#include "../Core/Application.h"

namespace WindGE
{
	class WIND_CORE_API Win32Window
	{
	public:
		Win32Window(HINSTANCE hInst);
		~Win32Window();

		bool init(Application* app, std::wstring titleName);
		void resize();
		int  run();

		virtual LRESULT msg_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		virtual void on_mouse_down(WPARAM /*btnState*/, int /*x*/, int /*y*/)	{}
		virtual void on_mouse_up(WPARAM /*btnState*/, int /*x*/, int /*y*/)		{}
		virtual void on_mouse_move(WPARAM /*btnState*/, int /*x*/, int /*y*/)	{}

	public:
		inline HINSTANCE	app_instance()	const { return __inst;	}
		inline HWND			app_hwnd()		const { return __hwnd;	}

	protected:
		bool _init_main_window();
		bool _init_app();

		void _calculate_frame_stats();

	private:
		HINSTANCE			__inst;
		HWND				__hwnd;
		bool				__is_paused;
		bool				__minimized;
		bool				__maximized;
		bool				__resizing;

		std::wstring		__title_name;
		int					__width;
		int					__height;
		int					__frame_count;

		Application*		__app;
		Timer				__timer;
	};
}

#endif // !__WIND32WINDOW_H__