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

#include "mask-resource-mesh.h"
#define NOMINMAX
#include "exceptions.h"
#include "plugin.h"
#include "utils.h"
#include <sstream>
#include <iterator>
#include <algorithm>
#include <unordered_map>
#include <tiny_obj_loader.h>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/util/platform.h>
	#include <libobs/obs-module.h>
	#pragma warning( pop )
}



static const char* const S_DATA = "data";
static const char* const S_VERTEX_BUFFER = "vertex-buffer";
static const char* const S_INDEX_BUFFER = "index-buffer";

Mask::Resource::Mesh::Mesh(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name) {

	// We could be an embedded OBJ file, or raw geometry

	// Raw geometry?
	if (obs_data_has_user_value(data, S_VERTEX_BUFFER)) {
		// Vertex Buffer
		const char* vertex64data = obs_data_get_string(data, S_VERTEX_BUFFER);
		if (vertex64data[0] == '\0') {
			PLOG_ERROR("Mesh '%s' has empty vertex data.", name.c_str());
			throw std::logic_error("Mesh has empty vertex data.");
		}
		std::vector<uint8_t> decodedVertices;
		base64_decode(vertex64data, decodedVertices);
		// add extra to buffer size to allow for alignment
		size_t vertBuffSize = zlib_size(decodedVertices) + 16;
		uint8_t* buffer = new uint8_t[vertBuffSize];
		zlib_decode(decodedVertices, (uint8_t*)ALIGNED(buffer));

		// Index Buffer
		if (!obs_data_has_user_value(data, S_INDEX_BUFFER)) {
			PLOG_ERROR("Mesh '%s' has no index buffer.", name.c_str());
			throw std::logic_error("Mesh has index buffer.");
		}
		const char* index64data = obs_data_get_string(data, S_INDEX_BUFFER);
		if (index64data[0] == '\0') {
			PLOG_ERROR("Mesh '%s' has empty index buffer data.", name.c_str());
			throw std::logic_error("Mesh has empty index buffer data.");
		}
		std::vector<uint8_t> decodedIndices;
		base64_decodeZ(index64data, decodedIndices);
		size_t numIndices = decodedIndices.size() / sizeof(uint32_t);

		// Make Buffers
		m_VertexBuffer = std::make_shared<GS::VertexBuffer>(buffer);
		m_IndexBuffer = std::make_shared<GS::IndexBuffer>
			((uint32_t*)decodedIndices.data(), numIndices);
	}
	
	// OBJ data?
	else {
		if (!obs_data_has_user_value(data, S_DATA)) {
			PLOG_ERROR("Mesh '%s' has no data.", name.c_str());
			throw std::logic_error("Mesh has no data.");
		}
		const char* base64data = obs_data_get_string(data, S_DATA);
		if (base64data[0] == '\0') {
			PLOG_ERROR("Mesh '%s' has empty data.", name.c_str());
			throw std::logic_error("Mesh has empty data.");
		}
		const char* tempFile = Utils::Base64ToTempFile(base64data);
		LoadObj(tempFile);
		Utils::DeleteTempFile(tempFile);
	}

	// calculate center
	vec3 center;
	vec3_zero(&center);
	vec3* v = m_VertexBuffer->get_data()->points;
	size_t numV = m_VertexBuffer->size();
	for (size_t i = 0; i < numV; i++, v++) {
		vec3_add(&center, &center, v);
	}
	vec3_divf(&center, &center, (float)numV);
	vec4_set(&m_center, center.x, center.y, center.z, 1.0f);
} 

Mask::Resource::Mesh::Mesh(Mask::MaskData* parent, std::string name, std::string file)
	: IBase(parent, name) {
	LoadObj(file);
}

Mask::Resource::Mesh::~Mesh() {}

Mask::Resource::Type Mask::Resource::Mesh::GetType() {
	return Mask::Resource::Type::Mesh;
}

void Mask::Resource::Mesh::Update(Mask::Part* part, float time) {
	UNUSED_PARAMETER(part);
	UNUSED_PARAMETER(time);
	return;
}

void Mask::Resource::Mesh::Render(Mask::Part* part) {
	UNUSED_PARAMETER(part);
	gs_load_vertexbuffer(m_VertexBuffer->get());
	gs_load_indexbuffer(m_IndexBuffer->get());
	gs_draw(gs_draw_mode::GS_TRIS, 0, (uint32_t)m_IndexBuffer->size());

	return;
}

static std::string GetBaseDir(const std::string &filepath) {
	if (filepath.find_last_of("/\\") != std::string::npos)
		return filepath.substr(0, filepath.find_last_of("/\\"));
	return "";
}

void Mask::Resource::Mesh::LoadObj(std::string file) {
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	tinyobj::attrib_t attrib;
	std::string err;
	std::string base_dir = GetBaseDir(file);
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str(), base_dir.c_str())) {
		blog(LOG_ERROR, "Unable to load OBJ file: %s", file.c_str());
		blog(LOG_ERROR, err.c_str());
		return;
	}

	// Create GPU mesh from tinyobj format.
	char keybuff[128];
	std::vector<GS::Vertex> vertices;
	std::vector<std::string> vertexKeys;
	std::vector<uint32_t> indices;
	for (size_t i = 0; i < shapes.size(); i++) {
		tinyobj::shape_t& shape = shapes[i];
		for (size_t j = 0; j < shape.mesh.indices.size(); j++) {
			tinyobj::index_t& index = shape.mesh.indices[j];

			snprintf(keybuff, sizeof(keybuff), "%d-%d-%d", index.vertex_index, index.normal_index,
				index.texcoord_index);
			std::string key = keybuff;

			size_t idx = 0;
			for (idx = 0; idx < vertexKeys.size(); idx++)
				if (vertexKeys[idx] == key)
					break;
		
			if (idx >= vertexKeys.size()) {
				GS::Vertex vtx;
				if (index.vertex_index == -1)
					break; 
				vtx.position.x = attrib.vertices[3 * index.vertex_index + 0];
				vtx.position.y = -attrib.vertices[3 * index.vertex_index + 1];
				vtx.position.z = attrib.vertices[3 * index.vertex_index + 2];
				if (index.normal_index != -1 && attrib.normals.size() > 0) {
					vtx.normal.x = attrib.normals[3 * index.normal_index + 0];
					vtx.normal.y = -attrib.normals[3 * index.normal_index + 1];
					vtx.normal.z = attrib.normals[3 * index.normal_index + 2];
				} else {
					vec3_norm(&vtx.normal, &vtx.position); // Normal from Vertex position.
				}
				if (index.texcoord_index != -1 && attrib.texcoords.size() > 0) {
					vtx.uv[0].x = attrib.texcoords[2 * index.texcoord_index + 0];
					vtx.uv[0].y = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
				}
				vertices.push_back(vtx);
				vertexKeys.push_back(key);
				idx = vertices.size() - 1;
			}
			indices.emplace_back((uint32_t)idx);
		}
	}

	// Create Vertex Buffer
	m_VertexBuffer = std::make_shared<GS::VertexBuffer>(vertices);

	/*
	// Calculate tangents
	gs_vb_data* vbdata = m_VertexBuffer->get_data();
	for (size_t i = 0; i < indices.size() / 3; i += 3) {
		uint32_t idx0 = indices[i + 0];
		uint32_t idx1 = indices[i + 1];
		uint32_t idx2 = indices[i + 2];

		vertices[idx0].tangent = CalculateTangent(vbdata, idx0, idx1, idx2);
		vertices[idx1].tangent = CalculateTangent(vbdata, idx1, idx2, idx0);
		vertices[idx2].tangent = CalculateTangent(vbdata, idx2, idx0, idx1);
	}
	*/
	
	// Create Index Buffer
	m_IndexBuffer = std::make_shared<GS::IndexBuffer>((uint32_t)indices.size());
	m_IndexBuffer->resize(indices.size());
	for (size_t idx = 0; idx < indices.size(); idx++) {
		m_IndexBuffer->at(idx) = indices[idx];
	}
}


// CalculateTangent
//
// https://learnopengl.com/#!Advanced-Lighting/Normal-Mapping
//
//
vec3 Mask::Resource::Mesh::CalculateTangent(gs_vb_data* vbdata, 
	int i1, int i2, int i3) {

	const vec3* v1 = &(vbdata->points[i1]);
	const vec3* v2 = &(vbdata->points[i2]);
	const vec3* v3 = &(vbdata->points[i3]);

	float *p = (float*)vbdata->tvarray[0].array;
	const vec2* uv1 = (vec2*)(p + (i1 * 2));
	const vec2* uv2 = (vec2*)(p + (i2 * 2));
	const vec2* uv3 = (vec2*)(p + (i3 * 2));

	vec3 edge1, edge2;
	vec3_sub(&edge1, v2, v1);
	vec3_sub(&edge2, v3, v1);

	vec2 deltaUV1, deltaUV2;
	vec2_sub(&deltaUV1, uv2, uv1);
	vec2_sub(&deltaUV2, uv3, uv1);

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	vec3 tangent;
	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

	vec3 normalized;
	vec3_norm(&normalized, &tangent);

	return normalized;
}

