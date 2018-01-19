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
#include "exceptions.h"
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/obs.h>
	#pragma warning( pop )
}

GS::VertexBuffer::VertexBuffer(uint8_t* raw)
 : m_vb_data(nullptr), m_vertexbuffer(nullptr), m_raw(nullptr) {
	m_raw = raw;
	gs_vb_data* vbdata = (gs_vb_data*)ALIGNED(raw);

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

	// create the gs vertex buffer
	obs_enter_graphics();
	m_vertexbuffer = gs_vertexbuffer_create(vbdata, 0);
	obs_leave_graphics();
} 


GS::VertexBuffer::VertexBuffer(const std::vector<GS::Vertex>& verts)
 : m_vb_data(nullptr), m_vertexbuffer(nullptr), m_raw(nullptr) {

	// Make a libOBS vertex buffer
	m_vb_data = gs_vbdata_create();
	m_vb_data->num = verts.size();
	m_vb_data->num_tex = 8;
	m_vb_data->points = (vec3*)bmalloc(sizeof(vec3) * verts.size());
	m_vb_data->normals = (vec3*)bmalloc(sizeof(vec3) * verts.size());
	m_vb_data->tangents = (vec3*)bmalloc(sizeof(vec3) * verts.size());
	m_vb_data->colors = (uint32_t*)bmalloc(sizeof(uint32_t) * verts.size());
	m_vb_data->tvarray = (gs_tvertarray*)bmalloc(sizeof(gs_tvertarray) * 8);
	for (int i = 0; i < 8; i++) {
		m_vb_data->tvarray[i].width = 4;
		m_vb_data->tvarray[i].array = bmalloc(sizeof(float) * 4 * verts.size());
	}

	// copy in the verts
	for (size_t i = 0; i < verts.size(); i++) {
		const GS::Vertex& vtx = verts[i];
		vec3_copy(&m_vb_data->points[i], &vtx.position);
		vec3_copy(&m_vb_data->normals[i], &vtx.normal);
		vec3_copy(&m_vb_data->tangents[i], &vtx.tangent);
		float* uvs = (float*)(m_vb_data->tvarray[0].array);
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
	if (m_vb_data)
		gs_vbdata_destroy(m_vb_data);
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