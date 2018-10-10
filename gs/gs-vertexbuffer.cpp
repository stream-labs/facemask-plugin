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

#include "gs-vertexbuffer.h"
#include "plugin/utils.h"
#include "plugin/exceptions.h"
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/obs.h>
	#pragma warning( pop )
}

static void init_vbdata(gs_vb_data* vbd, size_t num_verts) {
	vbd->num = num_verts;
	vbd->num_tex = 8;
	vbd->points = (vec3*)bmalloc(sizeof(vec3) * num_verts);
	vbd->normals = (vec3*)bmalloc(sizeof(vec3) * num_verts);
	vbd->tangents = (vec3*)bmalloc(sizeof(vec3) * num_verts);
	vbd->colors = (uint32_t*)bmalloc(sizeof(uint32_t) * num_verts);
	vbd->tvarray = (gs_tvertarray*)bmalloc(sizeof(gs_tvertarray) * 8);
	for (int i = 0; i < 8; i++) {
		vbd->tvarray[i].width = 4;
		vbd->tvarray[i].array = bmalloc(sizeof(float) * 4 * num_verts);
	}
}

GS::VertexBuffer::VertexBuffer(uint8_t* raw)
 : m_vb_data(nullptr), m_vertexbuffer(nullptr), m_raw(nullptr) {
	m_raw = raw;
	gs_vb_data* vbdata = (gs_vb_data*)ALIGN_16(raw);

	// sanity check
	if (vbdata->num_tex < 0 || vbdata->num_tex > 8) {
		// Bail...bad data
		blog(LOG_ERROR, "[Face Mask] Vertex Buffer is old format. Skipping. Mask will not render correctly, if at all.");
		return;
	}

	// classic memory image dereferencing
	//
	// For more info on how this data was created, see the Maskmaker
	// code that created it:
	//
	// facemask-plugin\tools\MaskMaker\command_import.cpp : GSVertexBuffer::get_data/size
	//
	size_t top = (size_t)vbdata;
	vbdata->points = (vec3*)((size_t)(vbdata->points) + top);
	vbdata->normals = (vec3*)((size_t)(vbdata->normals) + top);
	vbdata->tangents = (vec3*)((size_t)(vbdata->tangents) + top);
	vbdata->colors = (uint32_t*)((size_t)(vbdata->colors) + top);
	vbdata->tvarray = (gs_tvertarray*)((size_t)(vbdata->tvarray) + top);
	for (int i = 0; i < vbdata->num_tex; i++) {
		vbdata->tvarray[i].array = (void*)((size_t)(vbdata->tvarray[i].array) + top);
	}

	// And now re-allocate the whole fucking thing
	// (for some reason, gs_vertexbuffer_destroy() now deletes the vb data :P )
	//
	gs_vb_data* vbd = gs_vbdata_create();
	init_vbdata(vbd, vbdata->num);
	memcpy(vbd->points, vbdata->points, sizeof(vec3) * vbdata->num);
	if (vbdata->normals)
		memcpy(vbd->normals, vbdata->normals, sizeof(vec3) * vbdata->num);
	if (vbdata->tangents)
		memcpy(vbd->tangents, vbdata->tangents, sizeof(vec3) * vbdata->num);
	if (vbdata->colors)
		memcpy(vbd->colors, vbdata->colors, sizeof(uint32_t) * vbdata->num);
	for (int i = 0; i < vbdata->num_tex; i++) {
		memcpy(vbd->tvarray[i].array, vbdata->tvarray[i].array, sizeof(float) * vbd->tvarray[i].width * vbdata->num);
	}

	// create the gs vertex buffer
	obs_enter_graphics();
	m_vertexbuffer = gs_vertexbuffer_create(vbd, 0);
	obs_leave_graphics();
} 


GS::VertexBuffer::VertexBuffer(const std::vector<GS::Vertex>& verts)
 : m_vb_data(nullptr), m_vertexbuffer(nullptr), m_raw(nullptr) {

	// Make a libOBS vertex buffer
	m_vb_data = gs_vbdata_create();
	init_vbdata(m_vb_data, verts.size());

	// copy in the verts
	float* uvs = (float*)(m_vb_data->tvarray[0].array);
	for (size_t i = 0; i < verts.size(); i++) {
		const GS::Vertex& vtx = verts[i];
		vec3_copy(&m_vb_data->points[i], &vtx.position);
		vec3_copy(&m_vb_data->normals[i], &vtx.normal);
		vec3_copy(&m_vb_data->tangents[i], &vtx.tangent);
		uvs[i * 4 + 0] = vtx.uv[0].x;
		uvs[i * 4 + 1] = vtx.uv[0].y;
	}

	// create the gs vertex buffer
	obs_enter_graphics();
	m_vertexbuffer = gs_vertexbuffer_create(m_vb_data, 0);
	obs_leave_graphics();
}


GS::VertexBuffer::~VertexBuffer() {
	if (m_raw)
		delete[] m_raw;

	obs_enter_graphics();
	// nope, should be destroyed below
	//if (m_vb_data)
	//	gs_vbdata_destroy(m_vb_data);
	if (m_vertexbuffer)
		gs_vertexbuffer_destroy(m_vertexbuffer);
	obs_leave_graphics();
}


gs_vertbuffer_t* GS::VertexBuffer::get() {
	return m_vertexbuffer;
}


gs_vb_data* GS::VertexBuffer::get_data() { 
	if (m_vb_data)
		return m_vb_data;
	if (m_vertexbuffer)
		return gs_vertexbuffer_get_data(m_vertexbuffer);
	return nullptr;
}


size_t GS::VertexBuffer::size() 
{ 
	return get_data() ? get_data()->num : 0;
}
