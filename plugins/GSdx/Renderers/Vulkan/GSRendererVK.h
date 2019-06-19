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

#pragma once

#include "Renderers/Common/GSRenderer.h"

class GSRendererVK : public GSRenderer
{
	class GSVertexTraceVK : public GSVertexTrace
	{
	public:
		GSVertexTraceVK(const GSState* state) : GSVertexTrace(state) {}
	};

protected:
	void Draw()
	{
	}

	GSTexture* GetOutput(int i, int& y_offset)
	{
		return NULL;
	}

public:
	GSRendererVK() 
		: GSRenderer() 
	{
	}
};
