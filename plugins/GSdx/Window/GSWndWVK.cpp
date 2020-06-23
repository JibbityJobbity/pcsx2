/*
 *	Copyright (C) 2019 PCSX2 Dev Team
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
#include "stdafx.h"
#include "GSWndWVK.h"

#ifdef _WIN32

void* GSWndWVK::GetDisplay()
{
	return (void*)m_hWnd;
}

bool GSWndWVK::Attach(void* handle, bool managed)
{
	m_managed = managed;
	m_hWnd = (HWND)handle;

	return true;
}

bool GSWndWVK::Create(const std::string& title, int w, int h)
{
	if(m_hWnd) return false;

	m_managed = true;

	WNDCLASS wc;

	memset(&wc, 0, sizeof(wc));

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = theApp.GetModuleHandle();
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = "GSWndWVK";

	if (!GetClassInfo(wc.hInstance, wc.lpszClassName, &wc))
	{
		if (!RegisterClass(&wc))
		{
			return false;
		}
	}

	DWORD style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW | WS_BORDER;

	GSVector4i r;

	GetWindowRect(GetDesktopWindow(), r);

	// Old GSOpen ModeWidth and ModeHeight are not necessary with this.
	bool remote = !!GetSystemMetrics(SM_REMOTESESSION);

	if (w <= 0 || h <= 0 || remote)
	{
		w = r.width() / 3;
		h = r.width() / 4;

		if (!remote)
		{
			w *= 2;
			h *= 2;
		}
	}

	r.left = (r.left + r.right - w) / 2;
	r.top = (r.top + r.bottom - h) / 2;
	r.right = r.left + w;
	r.bottom = r.top + h;

	AdjustWindowRect(r, style, FALSE);

	m_hWnd = CreateWindow(wc.lpszClassName, title.c_str(), style, r.left, r.top, r.width(), r.height(), NULL, NULL, wc.hInstance, (LPVOID)this);

	if (m_hWnd == NULL) return false;

	return true;
}

bool GSWndWVK::SetWindowText(const char* title)
{
	if (!m_managed) return false;

	// Used by GSReplay.
	::SetWindowText(m_hWnd, title);

	return true;
}

void GSWndWVK::Show()
{
	if (!m_managed) return;

	// Used by GSReplay
	SetForegroundWindow(m_hWnd);
	ShowWindow(m_hWnd, SW_SHOWNORMAL);
	UpdateWindow(m_hWnd);
}

void GSWndWVK::Hide()
{
}

LRESULT CALLBACK GSWndWVK::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		// This takes place before GSClose, so don't destroy the Window so we can clean up.
		ShowWindow(hWnd, SW_HIDE);
		// DestroyWindow(hWnd);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

GSVector4i GSWndWVK::GetClientRect()
{
	GSVector4i r;

	::GetClientRect(m_hWnd, r);

	return r;
}

#endif 