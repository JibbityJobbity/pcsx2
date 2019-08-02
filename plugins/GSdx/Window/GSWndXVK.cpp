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

#include "GSWndXVK.h"

#ifdef __linux__
void* GSWndXVK::GetHandle()
{
	return (void*)m_XWindow; 
}

void* GSWndXVK::GetDisplay()
{
	return (void*)m_XDisplay;
}

bool GSWndXVK::Attach(void* handle, bool managed)
{
	m_managed = managed;
	m_XWindow = *(Window*)handle;
	m_XDisplay = XOpenDisplay(NULL);

	return true;
}

bool GSWndXVK::Create(const std::string& title, int w, int h) 
{
	if (m_XWindow)
		throw GSDXRecoverableError();
	
	if (w <= 0 || w <= 0) {
		w = theApp.GetConfigI("ModeWidth");
		h = theApp.GetConfigI("ModeHeight");
	}

	m_managed = true;
	// Something was mentioned in GSWndOGL.cpp about this line
	// "note this part must be only executed when replaying .gs debug file"
	m_XDisplay = XOpenDisplay(NULL);

	m_XWindow = XCreateSimpleWindow(m_XDisplay, DefaultRootWindow(m_XDisplay), 0, 0, w, h, 0, 0, 0);
	XMapWindow(m_XDisplay, m_XWindow);

	if (m_XWindow == 0)
		throw GSDXRecoverableError();
	
	m_w = w;
	m_h = h;

	return true;
}

void GSWndXVK::Detach()
{
	if (m_XDisplay) {
		XCloseDisplay(m_XDisplay);
		m_XDisplay = NULL;
	}
}

bool GSWndXVK::SetWindowText(const char* title)
{
	if (!m_managed) return true;

	XTextProperty prop;

	memset(&prop, 0, sizeof(prop));

	char* ptitle = (char*)title;
	if (XStringListToTextProperty(&ptitle, 1, &prop)) {
		XSetWMName(m_XDisplay, m_XWindow, &prop);
	}

	XFree(prop.value);
	XFlush(m_XDisplay);

	return true;
}

void GSWndXVK::Show()
{
	XMapRaised(m_XDisplay, m_XWindow);
	XFlush(m_XDisplay);
}

void GSWndXVK::Hide()
{
	XUnmapWindow(m_XDisplay, m_XWindow);
	XFlush(m_XDisplay);
}
#endif