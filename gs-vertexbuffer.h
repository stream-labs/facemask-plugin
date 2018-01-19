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
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/graphics/graphics.h>
	#include <libobs/obs.h>
	#pragma warning( pop )
}

/* Stripped-down version of GS::VertexBuffer
 *
 * yes, this really is all there is to it. 
 *
 * the meshes load a block of memory from the json data
 * with all the vertex data in it, so all we need to do
 * is wrap it up.
 *
 * it is not our responsibility to delete the gs_vb_data 
 * we were constructed with, either.
 *
 */
namespace GS {
	class VertexBuffer {
	public:
		VertexBuffer(gs_vb_data* d) {
			m_vertexbufferdata = d;
			obs_enter_graphics();
			m_vertexbuffer = gs_vertexbuffer_create(m_vertexbufferdata, 0);
			obs_leave_graphics();
		}
		~VertexBuffer() {
			obs_enter_graphics();
			gs_vertexbuffer_destroy(m_vertexbuffer);
			obs_leave_graphics();
		}

		gs_vertbuffer_t* get() {
			return get(true);
		}
		gs_vertbuffer_t* get(bool refreshGPU) {
			if (refreshGPU)
				gs_vertexbuffer_flush(m_vertexbuffer);
			return m_vertexbuffer;
		}
		gs_vb_data* get_data() { return m_vertexbufferdata; }
		size_t size() { return m_vertexbufferdata ? m_vertexbufferdata->num : 0; }

	protected:
		gs_vb_data* m_vertexbufferdata;
		gs_vertbuffer_t* m_vertexbuffer;

	};
}
