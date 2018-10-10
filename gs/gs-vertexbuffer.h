/*
 * Face Masks for SlOBS
 * Copyright (C) 2017 General Workings Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#pragma once
#include <inttypes.h>
#include <vector>
#include "gs-vertex.h"
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/graphics/graphics.h>
	#include <libobs/obs.h>
	#pragma warning( pop )
}

namespace GS {
	class VertexBuffer {
	public:
		VertexBuffer(uint8_t* raw);
		VertexBuffer(const std::vector<GS::Vertex>& verts);
		~VertexBuffer();

		gs_vertbuffer_t* get();
		gs_vb_data* get_data();
		size_t size();

	protected:
		gs_vb_data*			m_vb_data;
		gs_vertbuffer_t*	m_vertexbuffer;
		uint8_t*			m_raw;
	};
}
