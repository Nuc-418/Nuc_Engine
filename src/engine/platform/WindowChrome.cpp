// WindowChrome implementation. See WindowChrome.h.

#include "engine/platform/WindowChrome.h"

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <windows.h>
#include <windowsx.h>   // GET_X_LPARAM / GET_Y_LPARAM
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace
{
	WNDPROC g_originalProc = nullptr;

	// Caption layout published by the editor each frame (client pixels).
	float g_barHeight = 0.0f;
	float g_menuRight = 0.0f;
	float g_buttonsLeft = 1e9f;

	// Grab band for edge/corner resizing, in pixels.
	const int kResizeBorder = 6;

	bool WindowIsMaximized(HWND hwnd)
	{
		WINDOWPLACEMENT wp = { sizeof(wp) };
		return GetWindowPlacement(hwnd, &wp) && wp.showCmd == SW_SHOWMAXIMIZED;
	}

	LRESULT CALLBACK ChromeProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_NCCALCSIZE:
			// Returning 0 with wParam==TRUE removes the standard window frame,
			// so the client area covers the whole window (including where the
			// title bar used to be). We keep WS_THICKFRAME/WS_CAPTION on the
			// window, so move/resize/snap/maximize still work natively.
			if (wParam == TRUE)
			{
				// A maximized borderless window is sized by Windows to the work
				// area *plus* the invisible resize frame, so that frame band (top
				// included) spills off-screen and clips our title/menu bar. Inset
				// the client by the frame thickness while maximized so the top bar
				// stays fully on-screen; the taskbar remains visible because the
				// default maximized size already respects the work area.
				if (WindowIsMaximized(hwnd))
				{
					NCCALCSIZE_PARAMS* params = (NCCALCSIZE_PARAMS*)lParam;
					int frameX = GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
					int frameY = GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
					params->rgrc[0].left   += frameX;
					params->rgrc[0].right  -= frameX;
					params->rgrc[0].top    += frameY;
					params->rgrc[0].bottom -= frameY;
				}
				return 0;
			}
			break;

		case WM_NCHITTEST:
		{
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			ScreenToClient(hwnd, &pt);
			RECT rc;
			GetClientRect(hwnd, &rc);

			// Edge/corner resize bands (not while maximized).
			if (!WindowIsMaximized(hwnd))
			{
				bool left = pt.x < kResizeBorder;
				bool right = pt.x >= rc.right - kResizeBorder;
				bool top = pt.y < kResizeBorder;
				bool bottom = pt.y >= rc.bottom - kResizeBorder;

				if (top && left)     return HTTOPLEFT;
				if (top && right)    return HTTOPRIGHT;
				if (bottom && left)  return HTBOTTOMLEFT;
				if (bottom && right) return HTBOTTOMRIGHT;
				if (left)            return HTLEFT;
				if (right)           return HTRIGHT;
				if (top)             return HTTOP;
				if (bottom)          return HTBOTTOM;
			}

			// Title bar: the strip between the menu items and the window
			// buttons drags the window; everything else stays clickable.
			if (pt.y < (LONG)g_barHeight &&
			    pt.x > (LONG)g_menuRight && pt.x < (LONG)g_buttonsLeft)
				return HTCAPTION;

			return HTCLIENT;
		}
		}

		return CallWindowProc(g_originalProc, hwnd, msg, wParam, lParam);
	}
}

namespace WindowChrome
{
	void Install(GLFWwindow* window)
	{
		HWND hwnd = glfwGetWin32Window(window);
		if (!hwnd || g_originalProc)
			return;

		g_originalProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)ChromeProc);

		// Force a non-client recalculation so the frame is dropped immediately.
		SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
		             SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	void SetTitleBar(float height, float menuRightEdge, float buttonsLeft)
	{
		g_barHeight = height;
		g_menuRight = menuRightEdge;
		g_buttonsLeft = buttonsLeft;
	}
}

#else // !_WIN32

namespace WindowChrome
{
	void Install(GLFWwindow*) {}
	void SetTitleBar(float, float, float) {}
}

#endif
