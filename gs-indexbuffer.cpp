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

#include "gs-indexbuffer.h"
#include "utils.h"
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/obs.h>
	#pragma warning( pop )
}

GS::IndexBuffer::IndexBuffer(const uint8_t* raw, size_t len) {
	// fucking realloc indices
	uint32_t* indices = (uint32_t*)bmalloc(sizeof(uint32_t) * len);
	memcpy(indices, (uint32_t*)ALIGN_16(raw), sizeof(uint32_t) * len);
	MakeBuffer(indices, len);
	m_raw = raw;
}

GS::IndexBuffer::IndexBuffer(const std::vector<uint32_t>& other)
	: IndexBuffer(other.data(), other.size()) {
}

GS::IndexBuffer::IndexBuffer(const uint32_t* buff, size_t len)
 : m_raw(nullptr) {
	MakeBuffer(buff, len);
}

void GS::IndexBuffer::MakeBuffer(const uint32_t* buff, size_t len) {
	m_numIndices = len;
	obs_enter_graphics();
	m_indexBuffer = gs_indexbuffer_create(gs_index_type::GS_UNSIGNED_LONG, (void*)buff, len, 0);
	obs_leave_graphics();
}

GS::IndexBuffer::~IndexBuffer() {
	obs_enter_graphics();
	//gs_indexbuffer_destroy(m_indexBuffer);
	obs_leave_graphics();
	if (m_raw)
		delete[] m_raw;
}

gs_indexbuffer_t* GS::IndexBuffer::get() {
	return m_indexBuffer;
}
