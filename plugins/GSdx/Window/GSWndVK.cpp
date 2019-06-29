/*
 *	Copyright (C) 2019 Hamish Elliott
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

#include "GSWndVK.h"

GSWndVK::GSWndVK()
{
	
}

GSWndVK::~GSWndVK() 
{
	
}

void* GSWndVK::GetDisplay()
{
	return m_NativeDisplay; 
}

#ifdef __linux__
Window GSWndVK::GetNativeWindow()
{
	return m_NativeWindow;
}
#endif

void GSWndVK::InitVulkan()
{

}

#ifdef __linux__
bool GSWndVK::Create(const std::string& title, int w, int h) 
{
	if (m_NativeWindow)
		throw GSDXRecoverableError();
	
	if (w <= 0 || w <= 0) {
		w = theApp.GetConfigI("ModeWidth");
		h = theApp.GetConfigI("ModeHeight");
	}

	m_managed = true;
	// Something was mentioned in GSWndOGL.cpp about this line
	// "note this part must be only executed when replaying .gs debug file"
	m_NativeDisplay = XOpenDisplay(NULL);

	m_NativeWindow = XCreateSimpleWindow(m_NativeDisplay, DefaultRootWindow(m_NativeDisplay), 0, 0, w, h, 0, 0, 0);
	XMapWindow(m_NativeDisplay, m_NativeWindow);

	if (m_NativeWindow == 0)
		throw GSDXRecoverableError();
	
	m_w = w;
	m_h = h;

	return true;
}
#endif

bool GSWndVK::Attach(void* handle, bool managed)
{
	m_managed = managed;
#ifdef __linux__
	m_NativeWindow = *(Window*)handle;
	m_NativeDisplay = XOpenDisplay(NULL);
#endif
#ifdef _WIN32
	m_NativeDisplay = (HWND)handle;
#endif
	InitVulkan();

	return true;
}

bool GSWndVK::SetWindowText(const char* title)
{
#ifdef __linux__
	if (!m_managed) return true;

	XTextProperty prop;

	memset(&prop, 0, sizeof(prop));

	char* ptitle = (char*)title;
	if (XStringListToTextProperty(&ptitle, 1, &prop)) {
		XSetWMName(m_NativeDisplay, m_NativeWindow, &prop);
	}

	XFree(prop.value);
	XFlush(m_NativeDisplay);
#endif

	return true;
}