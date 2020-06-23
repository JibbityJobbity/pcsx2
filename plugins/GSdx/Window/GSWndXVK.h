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
#pragma once
#ifdef __linux__

#include "GSWnd.h"
#include <X11/Xlib.h>
#include <GL/glx.h>

class GSWndXVK : public GSWnd
{
protected:
	int m_w, m_h;
	Window		m_XWindow;
	Display* 	m_XDisplay;
public:
	GSWndXVK(){};
	virtual ~GSWndXVK(){};
	void* GetHandle();
	void* GetDisplay();
	bool Attach(void* handle, bool managed = true);
	bool SetWindowText(const char* title);
	bool Create(const std::string& title, int w, int h);
	void Detach();
	void Show();
	void Hide();
	void HideFrame(){};
	GSVector4i GetClientRect();
};
#endif 