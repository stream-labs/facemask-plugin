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
#include "plugin/exceptions.h"
#include "plugin/plugin.h"
#include "plugin/utils.h"
#include <sstream>
#include <iterator>
#include <algorithm>
#include <unordered_map>
#include <tiny_obj_loader.h>
#include <opencv2/opencv.hpp>
#include "mask.h"
#include "mask-resource-model.h"

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
static const char* const S_CENTER = "center";

Mask::Resource::Mesh::Mesh(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name), m_part(nullptr) {

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
		m_rawVertices = new uint8_t[vertBuffSize];
		zlib_decode(decodedVertices, (uint8_t*)ALIGN_16(m_rawVertices));

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
		base64_decode(index64data, decodedIndices);
		size_t idxBuffSize = zlib_size(decodedIndices);
		m_numIndices = (int)(idxBuffSize / sizeof(uint32_t));
		// add extra to buffer size to allow for alignment
		m_rawIndices = new uint8_t[idxBuffSize + 16];
		zlib_decode(decodedIndices, (uint8_t*)ALIGN_16(m_rawIndices));
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
		m_tempFile = Utils::Base64ToTempFile(base64data);
	}

	// center?
	vec3 center;
	vec3_zero(&center);
	if (obs_data_has_user_value(data, S_CENTER)) {
		obs_data_get_vec3(data, S_CENTER, &center);
	}
	vec4_set(&m_center, center.x, center.y, center.z, 1.0f);
} 

Mask::Resource::Mesh::Mesh(Mask::MaskData* parent, std::string name, std::string file)
	: IBase(parent, name), m_part(nullptr) {
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

	if (m_VertexBuffer == nullptr) {
		// create GS resources
		if (m_tempFile.length() > 0) {
			LoadObj(m_tempFile);
			Utils::DeleteTempFile(m_tempFile);
			m_tempFile.clear();
		}
		else {
			m_VertexBuffer = std::make_shared<GS::VertexBuffer>(m_rawVertices);
			m_IndexBuffer = std::make_shared<GS::IndexBuffer>(m_rawIndices, m_numIndices);
		}
	}

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

	// Calculate tangents
	for (size_t i = 0; i < indices.size() / 3; i += 3) {
		uint32_t idx0 = indices[i + 0];
		uint32_t idx1 = indices[i + 1];
		uint32_t idx2 = indices[i + 2];

		vertices[idx0].tangent = CalculateTangent(vertices[idx0], vertices[idx1], vertices[idx2]);
		vertices[idx1].tangent = CalculateTangent(vertices[idx1], vertices[idx2], vertices[idx0]);
		vertices[idx2].tangent = CalculateTangent(vertices[idx2], vertices[idx0], vertices[idx1]);
	}

	// Create Vertex Buffer
	m_VertexBuffer = std::make_shared<GS::VertexBuffer>(vertices);

	// Create Index Buffer
	m_IndexBuffer = std::make_shared<GS::IndexBuffer>(indices);
}


static void vec2_from_vec4(vec2* v2, const vec4* v4) {
	v2->x = v4->x;
	v2->y = v4->y;
}

// CalculateTangent
//
// https://learnopengl.com/#!Advanced-Lighting/Normal-Mapping
//
//
vec3 Mask::Resource::Mesh::CalculateTangent(const GS::Vertex& v1,
	const GS::Vertex& v2, const GS::Vertex& v3) {

	vec3 edge1, edge2;
	vec3_sub(&edge1, &v2.position, &v1.position);
	vec3_sub(&edge2, &v3.position, &v1.position);

	vec2 deltaUV1, deltaUV2, uv1, uv2, uv3;
	vec2_from_vec4(&uv1, &v1.uv[0]);
	vec2_from_vec4(&uv2, &v2.uv[0]);
	vec2_from_vec4(&uv3, &v3.uv[0]);
	vec2_sub(&deltaUV1, &uv2, &uv1);
	vec2_sub(&deltaUV2, &uv3, &uv1);

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	vec3 tangent;
	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

	vec3 normalized;
	vec3_norm(&normalized, &tangent);

	return normalized;
}

bool Mask::Resource::Mesh::GetScreenExtents(gs_rect* r, int screen_width, int screen_height, float trsZ) {
	gs_vb_data* vb_data = nullptr;
	if (m_VertexBuffer)
		vb_data = m_VertexBuffer->get_data();
	if (vb_data) {
		// We need to find our transform
		matrix4 transform;
		matrix4_identity(&transform);

		// Find our model
		if (m_part == nullptr) {
			size_t numModels = m_parent->GetNumResources(Resource::Type::Model);
			for (int i = 0; i < numModels; i++) {
				std::shared_ptr<Mask::Resource::Model> mdl = std::dynamic_pointer_cast<Mask::Resource::Model>
					(m_parent->GetResource(Resource::Type::Model, i));
				if (mdl->GetMesh()->GetId() == GetId()) {

					// Found it! Now let's find our part
					size_t numParts = m_parent->GetNumParts();
					for (int j = 0; j < numParts; j++) {
						std::shared_ptr<Mask::Part> part = m_parent->GetPart(j);
						for (auto const& it : part->resources) {
							if (it->GetId() == mdl->GetId()) {
								m_part = part;
							}
						}
					}
				}
			}
		}
		if (m_part != nullptr) {
			matrix4_copy(&transform, &(m_part->global));
		}

		// get points
		std::vector<cv::Point3f> points;
		for (int i = 0; i < vb_data->num; i++) {
			vec4 p, pp;
			vec4_from_vec3(&p, &(vb_data->points[i]));
			p.w = 1.0f;
			vec4_transform(&pp, &p, &transform);
			points.emplace_back(cv::Point3f(pp.x, pp.y, pp.z));
		}

		// Approximate focal length.
		float focal_length = (float)screen_width;
		cv::Point2f center = cv::Point2f(screen_width / 2.0f, screen_height / 2.0f);
		cv::Mat camera_matrix =
			(cv::Mat_<float>(3, 3) <<
				focal_length, 0, center.x,
				0, focal_length, center.y,
				0, 0, 1);
		// We assume no lens distortion
		cv::Mat dist_coeffs = cv::Mat::zeros(4, 1, cv::DataType<float>::type);

		cv::Mat rot = cv::Mat::zeros(3, 1, cv::DataType<float>::type);
		cv::Mat trx = (cv::Mat_<float>(3, 1) << 0, 0, trsZ);

		// project
		std::vector<cv::Point2f> projpoints;
		cv::projectPoints(points, rot, trx, camera_matrix, dist_coeffs, projpoints);

		// get extents
		int x_min = screen_width;
		int x_max = 0;
		int y_min = screen_height;
		int y_max = 0;
		for (int i = 0; i < projpoints.size(); i++) {
			int x = (int)projpoints[i].x;
			int y = (int)projpoints[i].y;
			if (x < x_min) x_min = x;
			if (y < y_min) y_min = y;
			if (x > x_max) x_max = x;
			if (y > y_max) y_max = y;
		}

		// set rect
		r->x = x_min;
		r->y = y_min;
		r->cx = x_max - x_min;
		r->cy = y_max - y_min;

		return true;
	}
	return false;
}


