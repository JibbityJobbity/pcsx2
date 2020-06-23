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
#include "GSWnd.h"

#include <vector>
#include <set>
#include <limits>


class GSWndVK : public GSWnd
{
protected:
	int	m_w, m_h;
	bool	m_has_late_vsync;

public:
	GSWndVK(){};
	~GSWndVK(){};

	virtual bool Create(const std::string& title, int w, int h) = 0;
	virtual bool Attach(void* handle, bool managed = true) = 0;
	void Detach(){};
	virtual void* GetDisplay() = 0;
	virtual void* GetHandle() = 0;
	GSVector4i GetClientRect(){return GSVector4i();}
	virtual bool SetWindowText(const char* title) = 0;
	void Show(){};
	void Hide(){};
	void HideFrame(){};
}; 