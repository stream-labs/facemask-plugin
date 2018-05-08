/*
* Face Masks for SlOBS
* smll - streamlabs machine learning library
*
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
#include "OBSRenderer.hpp"

#pragma warning( push )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4201 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4505 )

#include <libobs/obs-module.h>
#include <libobs/graphics/matrix4.h>
#include <libobs/graphics/image-file.h>
#include <tiny_obj_loader.h>


#pragma warning( pop )

#include <iostream>
using namespace std;

#define VERTEX_BUFF_X_AXIS	(0)
#define VERTEX_BUFF_Y_AXIS	(1)
#define VERTEX_BUFF_Z_AXIS	(2)
#define VERTEX_BUFF_GLASSES	(3)
#define VERTEX_BUFF_MASK	(4)

namespace smll {

	
	static float FOVA(float aspect)
	{
		// field of view angle matched to focal length for solvePNP
		return 56.0f / aspect;
	}

	static int TEXUNIT = 0;

	OBSRenderer::OBSRenderer()
		: m_viewportWidth(0)
		, m_viewportHeight(0)
		, m_phongEffect(nullptr)
		, m_samplerState(nullptr) 
		, m_colorConversion(nullptr) {
		obs_enter_graphics();

		gs_sampler_info sinfo;
		sinfo.address_u = GS_ADDRESS_CLAMP;
		sinfo.address_v = GS_ADDRESS_CLAMP;
		sinfo.address_w = GS_ADDRESS_CLAMP;
		sinfo.filter = GS_FILTER_LINEAR;
		sinfo.border_color = 0;
		sinfo.max_anisotropy = 0;

		m_samplerState = gs_samplerstate_create(&sinfo);
		static const char* const fn = "effects/color_conversion.effect";
		char* filename = obs_module_file(fn);
		char* error;
		m_colorConversion = gs_effect_create_from_file(filename, &error);
		if (!m_colorConversion) {
			blog(LOG_ERROR, "Cannot open color_conversion.effect. %s", error);
		}
		bfree(filename);

		// load the phong shader effect
		char* phongFilename = obs_module_file("effects/phong.effect");
		m_phongEffect = gs_effect_create_from_file(phongFilename, &error);
		if (!m_phongEffect) {
			blog(LOG_ERROR, "Cannot load phong effect %s", error);
		}
		bfree(phongFilename);

		// make vertex buffers
		// x axis
		BeginVertexBuffer();
		gs_vertex3f(0, 0, 0);
		gs_vertex3f(1000, 0, 0);
		EndVertexBuffer();
		// y axis
		BeginVertexBuffer();
		gs_vertex3f(0, 0, 0);
		gs_vertex3f(0, 1000, 0);
		EndVertexBuffer();
		// z axis
		BeginVertexBuffer();
		gs_vertex3f(0, 0, 0);
		gs_vertex3f(0, 0, 1000);
		EndVertexBuffer();

		drawTexRender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

		obs_leave_graphics();
	}

	OBSRenderer::~OBSRenderer()	{
		obs_enter_graphics();
		if (m_phongEffect)
			gs_effect_destroy(m_phongEffect);
		gs_samplerstate_destroy(m_samplerState);
		for (int i = 0; i < m_vertexBuffers.size(); i++) {
			gs_vertexbuffer_destroy(m_vertexBuffers[i]);
		}
		gs_effect_destroy(m_colorConversion);
		gs_texrender_destroy(drawTexRender);
		obs_leave_graphics();
	}

	void OBSRenderer::SetDrawColor(unsigned char red, unsigned char green, 
		unsigned char blue, unsigned char alpha) {
		gs_effect_t    *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
		gs_eparam_t    *color = gs_effect_get_param_by_name(solid, "color");

		struct vec4 veccol;
		vec4_from_rgba(&veccol, MakeColor(red, green, blue, alpha));
		gs_effect_set_vec4(color, &veccol);
	}

	void OBSRenderer::SetViewport(int w, int h)	{
		m_viewportWidth = w;
		m_viewportHeight = h;
	}

	int OBSRenderer::LoadTexture(const ImageWrapper& image)	{
		int t = (int)m_textures.size();

		if ((image.w * image.getNumElems()) != image.getStride()) {
			throw std::invalid_argument(
				"OBS does not allow oddly strided textures");
		}

		gs_texture* tex = gs_texture_create(image.w, image.h, 
			SMLLToOBS(image.type), 0, (const uint8_t**)&image.data, 0);

		m_textures.push_back(tex);

		return t;
	}

	int	OBSRenderer::LoadTexture(const char* filename) {
		gs_texture_t *tex = gs_texture_create_from_file(filename);

		int t = (int)m_textures.size();

		if (tex)
			m_textures.push_back(tex);
		else
			t = -1;

		return t;
	}

	void OBSRenderer::UnLoadTexture(int texture) {
		if (texture < 0 || texture >= m_textures.size())
			return;

		gs_texture_destroy(m_textures[texture]);
		m_textures[texture] = nullptr;
	}

	void OBSRenderer::DrawFaces(const DetectionResults& faces) {
		gs_effect_t    *solid = obs_get_base_effect(OBS_EFFECT_SOLID);

		for (int i = 0; i < faces.length; i++) {
			SetDrawColor(255, 255, 0);
			DrawRect(faces[i].bounds);

			DrawLandmarks(faces[i].landmarks68, 0, 255, 0);

			/* 5 landmarks 
			SetDrawColor(255, 0, 255);
			drawLines(faces[i].landmarks5, FIVE_LANDMARK_EYE_RIGHT_OUTER, FIVE_LANDMARK_EYE_RIGHT_INNER);
			SetDrawColor(255, 0, 255);
			drawLines(faces[i].landmarks5, FIVE_LANDMARK_EYE_LEFT_OUTER, FIVE_LANDMARK_NOSE_BOTTOM);
			SetDrawColor(255, 0, 255);
			drawLine(faces[i].landmarks5, FIVE_LANDMARK_EYE_RIGHT_INNER, FIVE_LANDMARK_NOSE_BOTTOM);
			*/

			// set up projection
			gs_projection_push();
			float aspect = (float)m_viewportWidth / (float)m_viewportHeight;
			gs_perspective(FOVA(aspect), aspect, 0.1f, 20000.0f);

			// set up transform
			gs_matrix_push();
			SetTransform(faces[i]);

			// draw axis
			while (gs_effect_loop(solid, "Solid")) {
				SetDrawColor(255, 0, 0);
				gs_load_vertexbuffer(GetVertexBuffer(VERTEX_BUFF_X_AXIS));
				gs_load_indexbuffer(nullptr);
				gs_draw(GS_LINES, 0, 0);

				SetDrawColor(0, 255, 0);
				gs_load_vertexbuffer(GetVertexBuffer(VERTEX_BUFF_Y_AXIS));
				gs_load_indexbuffer(nullptr);
				gs_draw(GS_LINES, 0, 0);

				SetDrawColor(0, 0, 255);
				gs_load_vertexbuffer(GetVertexBuffer(VERTEX_BUFF_Z_AXIS));
				gs_load_indexbuffer(nullptr);
				gs_draw(GS_LINES, 0, 0);
			}

			gs_matrix_pop();
			gs_projection_pop();
		}
	}

	void OBSRenderer::DrawLandmarks(const dlib::point* points, 
		uint8_t r, uint8_t g, uint8_t b) {
		// landmarks
		SetDrawColor(r, g, b);
		drawLines(points, JAW_1, JAW_17);
		SetDrawColor(r, g, b);
		drawLines(points, EYEBROW_LEFT_1, EYEBROW_LEFT_5);
		SetDrawColor(r, g, b);
		drawLines(points, EYEBROW_RIGHT_1, EYEBROW_RIGHT_5);
		SetDrawColor(r, g, b);
		drawLines(points, NOSE_1, NOSE_TIP);
		SetDrawColor(r, g, b);
		drawLines(points, NOSE_TIP, NOSE_9, true);
		SetDrawColor(r, g, b);
		drawLines(points, EYE_LEFT_1, EYE_LEFT_6, true);
		SetDrawColor(r, g, b);
		drawLines(points, EYE_RIGHT_1, EYE_RIGHT_6, true);
		SetDrawColor(r, g, b);
		drawLines(points, MOUTH_OUTER_1, MOUTH_OUTER_12, true);
		SetDrawColor(r, g, b);
		drawLines(points, MOUTH_INNER_1, MOUTH_INNER_8, true);
	}

	void OBSRenderer::DrawRect(const dlib::rectangle& r) {
		// rect
		dlib::point p[4];
		p[0] = dlib::point(r.left(), r.top());
		p[1] = dlib::point(r.right(), r.top());
		p[2] = dlib::point(r.right(), r.bottom());
		p[3] = dlib::point(r.left(), r.bottom());
		drawLines(p, 0, 3, true);
	}

	void   OBSRenderer::DrawGlasses(const DetectionResult& face, int texture) {
		if (texture < 0 || texture >= m_textures.size())
			return;

		gs_projection_push();
		float aspect = (float)m_viewportWidth / (float)m_viewportHeight;
		gs_perspective(FOVA(aspect), aspect, 1, 20000);

		gs_matrix_push();
		SetTransform(face);

		gs_effect_t *eff = obs_get_base_effect(OBS_EFFECT_DEFAULT);

		gs_texture* tex = m_textures[texture];

		gs_set_cull_mode(GS_BACK);
		while (gs_effect_loop(eff, "Draw")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(eff, "image"), tex);
			gs_load_vertexbuffer(GetVertexBuffer(VERTEX_BUFF_GLASSES));
			gs_load_indexbuffer(nullptr);
			gs_load_texture(tex, TEXUNIT);
			gs_load_samplerstate(m_samplerState, TEXUNIT);
			gs_draw(GS_TRIS, 0, 0);
		}

		gs_matrix_pop();
		gs_projection_pop();
	}


	void   OBSRenderer::DrawMask(const DetectionResult& face, int texture)	{
		if (texture < 0 || texture >= m_textures.size())
			return;

		gs_projection_push();
		float aspect = (float)m_viewportWidth / (float)m_viewportHeight;
		gs_perspective(FOVA(aspect), aspect, 1, 20000);

		gs_matrix_push();
		SetTransform(face);

		gs_effect_t *eff = obs_get_base_effect(OBS_EFFECT_DEFAULT);

		gs_texture* tex = m_textures[texture];

		gs_set_cull_mode(GS_BACK);
		while (gs_effect_loop(eff, "Draw")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(eff, "image"), tex);
			gs_load_vertexbuffer(GetVertexBuffer(VERTEX_BUFF_MASK));
			gs_load_indexbuffer(nullptr);
			gs_load_texture(tex, TEXUNIT);
			gs_load_samplerstate(m_samplerState, TEXUNIT);
			gs_draw(GS_TRISTRIP, 0, 0);
		}

		gs_matrix_pop();
		gs_projection_pop();
	}

	void    OBSRenderer::DrawModel(const DetectionResult& face, int vertexbuffer, int texture) {
		gs_enable_depth_test(true);
		gs_set_cull_mode(GS_NEITHER);

		gs_projection_push();
		float aspect = (float)m_viewportWidth / (float)m_viewportHeight;
		gs_perspective(FOVA(aspect), aspect, 1000.0f, 5000.0f);

		gs_matrix_push();
		SetTransform(face);
		gs_matrix_translate3f(0.0f, -200.0f, 0.0f);

		gs_effect_t *eff = m_phongEffect;
		gs_texture* tex = m_textures[texture];

		matrix4 world;
		gs_matrix_get(&world);
		matrix4 normalMat(world);
		normalMat.t.x = 0.0f;
		normalMat.t.y = 0.0f;
		normalMat.t.z = 0.0f;
		normalMat.t.w = 1.0f;

		while (gs_effect_loop(eff, "Draw")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(eff, "image"), tex);
			gs_effect_set_matrix4(gs_effect_get_param_by_name(eff, "World"), &world);
			gs_effect_set_matrix4(gs_effect_get_param_by_name(eff, "NormalMat"), &normalMat);
			gs_load_vertexbuffer(GetVertexBuffer(vertexbuffer));
			gs_load_indexbuffer(nullptr);
			gs_load_texture(tex, TEXUNIT);
			gs_load_samplerstate(m_samplerState, TEXUNIT);
			gs_draw(GS_TRIS, 0, 0);
		}

		gs_matrix_pop();
		gs_projection_pop();
	}

	void    OBSRenderer::Draw2DMask(const DetectionResult& face, int texture) {
		if (texture < 0 || texture >= m_textures.size())
			return;

		gs_effect_t *eff = obs_get_base_effect(OBS_EFFECT_DEFAULT);

		gs_texture* tex = m_textures[texture];

		gs_sampler_info sinfo;
		sinfo.address_u = GS_ADDRESS_CLAMP;
		sinfo.address_v = GS_ADDRESS_CLAMP;
		sinfo.address_w = GS_ADDRESS_CLAMP;
		sinfo.filter = GS_FILTER_LINEAR;
		sinfo.border_color = 0;
		sinfo.max_anisotropy = 0;

		gs_matrix_push(); 

		float x = (float)(face.bounds.left() - 
			(face.bounds.width() / 2));
		float y = (float)(face.bounds.bottom() +
			(face.bounds.height() / 2));
		gs_matrix_translate3f(x, y, 0);
		gs_matrix_scale3f(1, -1, 1);

		gs_set_cull_mode(GS_NEITHER);
		while (gs_effect_loop(eff, "Draw")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(eff, "image"), tex);
			gs_draw_sprite(tex, GS_FLIP_V, face.bounds.width() * 2,
				face.bounds.height() * 2);
		}

		gs_matrix_pop();
	}

	void	OBSRenderer::BeginVertexBuffer() {
		gs_render_start(true);
	}

	int		OBSRenderer::EndVertexBuffer() {
		gs_vertbuffer_t *vertbuff = gs_render_save();
		int i = (int)m_vertexBuffers.size();
		m_vertexBuffers.push_back(vertbuff);
		return i;
	}

	gs_vertbuffer_t* OBSRenderer::GetVertexBuffer(int which) {
		if (which < 0 || which >= m_vertexBuffers.size())
			return nullptr;

		return m_vertexBuffers[which];
	}

	// Methods taken wholesale from the objview example in tinyobjloader
	//
	static std::string GetBaseDir(const std::string &filepath) {
		if (filepath.find_last_of("/\\") != std::string::npos)
			return filepath.substr(0, filepath.find_last_of("/\\"));
		return "";
	}
	static void CalcNormal(float N[3], float v0[3], float v1[3], float v2[3]) {
		float v10[3];
		v10[0] = v1[0] - v0[0];
		v10[1] = v1[1] - v0[1];
		v10[2] = v1[2] - v0[2];

		float v20[3];
		v20[0] = v2[0] - v0[0];
		v20[1] = v2[1] - v0[1];
		v20[2] = v2[2] - v0[2];

		N[0] = v20[1] * v10[2] - v20[2] * v10[1];
		N[1] = v20[2] * v10[0] - v20[0] * v10[2];
		N[2] = v20[0] * v10[1] - v20[1] * v10[0];

		float len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
		if (len2 > 0.0f) {
			float len = sqrtf(len2);

			N[0] /= len;
			N[1] /= len;
		}
	}

	int    OBSRenderer::LoadVertexBufferFromOBJ(const char* filename)
	{
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		tinyobj::attrib_t attrib;
		std::string err;
		std::string base_dir = GetBaseDir(filename);
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, base_dir.c_str()))
		{
			blog(LOG_ERROR, "Unable to load OBJ file: %s", filename);
			blog(LOG_ERROR, err.c_str());
			return -1;
		}

		float bmin[3], bmax[3];
		bmin[0] = bmin[1] = bmin[2] = std::numeric_limits<float>::max();
		bmax[0] = bmax[1] = bmax[2] = -std::numeric_limits<float>::max();

		// run through the verts first to get bounding box
		for (size_t s = 0; s < shapes.size(); s++) {
			for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++) {

				tinyobj::index_t idx0 = shapes[s].mesh.indices[3 * f + 0];
				tinyobj::index_t idx1 = shapes[s].mesh.indices[3 * f + 1];
				tinyobj::index_t idx2 = shapes[s].mesh.indices[3 * f + 2];

				// vertex position
				float v[3][3];
				for (int k = 0; k < 3; k++) {
					int f0 = idx0.vertex_index;
					int f1 = idx1.vertex_index;
					int f2 = idx2.vertex_index;
					assert(f0 >= 0);
					assert(f1 >= 0);
					assert(f2 >= 0);

					v[0][k] = attrib.vertices[3 * f0 + k];
					v[1][k] = attrib.vertices[3 * f1 + k];
					v[2][k] = attrib.vertices[3 * f2 + k];
					bmin[k] = std::min(v[0][k], bmin[k]);
					bmin[k] = std::min(v[1][k], bmin[k]);
					bmin[k] = std::min(v[2][k], bmin[k]);
					bmax[k] = std::max(v[0][k], bmax[k]);
					bmax[k] = std::max(v[1][k], bmax[k]);
					bmax[k] = std::max(v[2][k], bmax[k]);
				}
			}
		}

		float bcenter[3];
		bcenter[0] = (bmax[0] + bmin[0]);
		bcenter[1] = (bmax[1] + bmin[1]);
		bcenter[2] = (bmax[2] + bmin[2]);
		float bsize[3];
		bsize[0] = bmax[0] - bmin[0];
		bsize[1] = bmax[1] - bmin[1];
		bsize[2] = bmax[2] - bmin[2];
		float max = bsize[0];
		max = std::max(bsize[1], max);
		max = std::max(bsize[2], max);
		float scaleFactor = 1500.0f / max;

		BeginVertexBuffer();

		for (size_t s = 0; s < shapes.size(); s++) {
			for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++) {

				tinyobj::index_t idx0 = shapes[s].mesh.indices[3 * f + 0];
				tinyobj::index_t idx1 = shapes[s].mesh.indices[3 * f + 1];
				tinyobj::index_t idx2 = shapes[s].mesh.indices[3 * f + 2];

				// texture coords
				float tc[3][2];
				if (attrib.texcoords.size() > 0) {
					assert(attrib.texcoords.size() > 2 * idx0.texcoord_index + 1);
					assert(attrib.texcoords.size() > 2 * idx1.texcoord_index + 1);
					assert(attrib.texcoords.size() > 2 * idx2.texcoord_index + 1);
					tc[0][0] = attrib.texcoords[2 * idx0.texcoord_index];
					tc[0][1] = 1.0f - attrib.texcoords[2 * idx0.texcoord_index + 1];
					tc[1][0] = attrib.texcoords[2 * idx1.texcoord_index];
					tc[1][1] = 1.0f - attrib.texcoords[2 * idx1.texcoord_index + 1];
					tc[2][0] = attrib.texcoords[2 * idx2.texcoord_index];
					tc[2][1] = 1.0f - attrib.texcoords[2 * idx2.texcoord_index + 1];
				}
				else {
					tc[0][0] = 0.0f;
					tc[0][1] = 0.0f;
					tc[1][0] = 0.0f;
					tc[1][1] = 0.0f;
					tc[2][0] = 0.0f;
					tc[2][1] = 0.0f;
				}

				// vertex position
				float v[3][3];
				for (int k = 0; k < 3; k++) {
					int f0 = idx0.vertex_index;
					int f1 = idx1.vertex_index;
					int f2 = idx2.vertex_index;
					assert(f0 >= 0);
					assert(f1 >= 0);
					assert(f2 >= 0);

					v[0][k] = attrib.vertices[3 * f0 + k];
					v[1][k] = attrib.vertices[3 * f1 + k];
					v[2][k] = attrib.vertices[3 * f2 + k];

					// CENTER
					v[0][k] -= bcenter[k];
					v[1][k] -= bcenter[k];
					v[2][k] -= bcenter[k];

					// SCALE 
					v[0][k] *= scaleFactor;
					v[1][k] *= scaleFactor;
					v[2][k] *= scaleFactor;
				}

				// normal vector
				float n[3][3];
				if (attrib.normals.size() > 0) {
					int f0 = idx0.normal_index;
					int f1 = idx1.normal_index;
					int f2 = idx2.normal_index;
					assert(f0 >= 0);
					assert(f1 >= 0);
					assert(f2 >= 0);
					for (int k = 0; k < 3; k++) {
						n[0][k] = attrib.normals[3 * f0 + k];
						n[1][k] = attrib.normals[3 * f1 + k];
						n[2][k] = attrib.normals[3 * f2 + k];
					}
				}
				else {
					// compute geometric normal
					CalcNormal(n[0], v[0], v[1], v[2]);
					n[1][0] = n[0][0];
					n[1][1] = n[0][1];
					n[1][2] = n[0][2];
					n[2][0] = n[0][0];
					n[2][1] = n[0][1];
					n[2][2] = n[0][2];
				}

				// add triangle
				for (int k = 0; k < 3; k++) {

					// add vertex
					gs_texcoord(tc[k][0], tc[k][1], TEXUNIT);
					gs_normal3f(n[k][0], n[k][1], -n[k][2]);
					gs_vertex3f(v[k][0], v[k][1], -v[k][2]);
				}
			}
		}

		int vb = EndVertexBuffer();

		return vb;
	}

	void OBSRenderer::DestroyVertexBufffer(int which)
	{
		if (which < 0 || which >= m_vertexBuffers.size())
			return;

		gs_vertexbuffer_destroy(m_vertexBuffers[which]);

		m_vertexBuffers.erase(m_vertexBuffers.begin() + which);
	}


	gs_color_format	OBSRenderer::SMLLToOBS(smll::ImageType t) {
		switch (t) {
		case ImageType::IMAGETYPE_LUMA:
			return GS_R8;
		case ImageType::IMAGETYPE_RGBA:
			return GS_RGBA;
		case ImageType::IMAGETYPE_BGRA:
			return GS_BGRA;
		default:
			return GS_UNKNOWN;
		}
	}

	smll::ImageType	OBSRenderer::OBSToSMLL(gs_color_format f) {
		switch (f) {
		case GS_A8:
		case GS_R8:
			return ImageType::IMAGETYPE_LUMA;
		case GS_RGBA:
			return ImageType::IMAGETYPE_RGBA;
		case GS_BGRX:
		case GS_BGRA:
			return ImageType::IMAGETYPE_BGRA;
		default:
			return ImageType::IMAGETYPE_INVALID;
		}
	}

	uint32_t OBSRenderer::MakeColor(uint8_t a, uint8_t b, uint8_t g, 
		uint8_t r) {
		return (((uint32_t)r << 24) & 0xFF000000) |
			   (((uint32_t)g << 16) & 0x00FF0000) |
			   (((uint32_t)b << 8)  & 0x0000FF00) |
			   (((uint32_t)a << 0)  & 0x000000FF);
	}

	void OBSRenderer::SetTransform(const DetectionResult& face) {
		gs_matrix_identity();
		gs_matrix_translate3f((float)face.pose.translation[0],
			(float)face.pose.translation[1], (float)-face.pose.translation[2]);
		gs_matrix_rotaa4f((float)face.pose.rotation[0], (float)face.pose.rotation[1],
			(float)-face.pose.rotation[2], (float)-face.pose.rotation[3]);
	}

	void	OBSRenderer::drawLines(const dlib::point* points, int start,
		int end, bool closed) {
		// make vb
		gs_render_start(true);
		// verts
		for (int i = start; i <= end; i++) {
			gs_vertex2f((float)points[i].x(), (float)points[i].y());
		}
		if (closed) {
			gs_vertex2f((float)points[start].x(), (float)points[start].y());
		}
		gs_vertbuffer_t *vertbuff = gs_render_save();

		while (gs_effect_loop(obs_get_base_effect(OBS_EFFECT_SOLID), "Solid")) {
			gs_load_vertexbuffer(vertbuff);
			gs_load_indexbuffer(nullptr);
			gs_draw(GS_LINESTRIP, 0, 0);
		}
		gs_vertexbuffer_destroy(vertbuff);
	}

	void	OBSRenderer::drawLine(const dlib::point* points, int start,
		int end) {
		// make vb
		gs_render_start(true);
		// verts
		gs_vertex2f((float)points[start].x(), (float)points[start].y());
		gs_vertex2f((float)points[end].x(), (float)points[end].y());
		gs_vertbuffer_t *vertbuff = gs_render_save();

		while (gs_effect_loop(obs_get_base_effect(OBS_EFFECT_SOLID), "Solid")) {
			gs_load_vertexbuffer(vertbuff);
			gs_load_indexbuffer(nullptr);
			gs_draw(GS_LINESTRIP, 0, 0);
		}
		gs_vertexbuffer_destroy(vertbuff);
	}

	void OBSRenderer::SpriteTexRender(gs_texture_t* texture,
		gs_texrender_t* texrender, int width, int height) {
		gs_enable_depth_test(false);
		gs_set_cull_mode(GS_NEITHER);

		gs_matrix_push();
		gs_projection_push();
		gs_viewport_push();

		gs_matrix_identity();
		gs_set_viewport(0, 0, width, height);
		gs_ortho(0.0f, (float)width, 0.0f, (float)height, 0.0f, 100.0f);

		// Begin rendering to a texture
		gs_texrender_reset(texrender);
		if (gs_texrender_begin(texrender, width, height)) {
			// clear
			vec4 black;
			vec4_zero(&black);
			gs_clear(GS_CLEAR_COLOR, &black, 0, 0);

			while (gs_effect_loop(m_colorConversion, "RGBToLuma")) {
				gs_effect_set_texture(gs_effect_get_param_by_name(m_colorConversion,
					"image"), texture);
				gs_draw_sprite(texture, 0, width, height);
			}

			gs_texrender_end(texrender);
		}

		gs_matrix_pop();
		gs_viewport_pop();
		gs_projection_pop();
	}

	gs_texture*	OBSRenderer::RenderTextToTexture(const std::string& text,
		int tex_width, int tex_height, const OBSFont& font) {

		std::vector<std::string> lines = font.BreakIntoLines(text, tex_width);

		return RenderTextToTexture(lines, tex_width, tex_height, font);
	}

	gs_texture*	OBSRenderer::RenderTextToTexture(const std::vector<std::string>& lines,
		int tex_width, int tex_height, const OBSFont& font) {

		gs_matrix_push();
		gs_projection_push();
		gs_viewport_push();

		gs_matrix_identity();
		gs_set_viewport(0, 0, tex_width, tex_height);
		gs_ortho(0.0f, (float)tex_width, 0.0f, (float)tex_height, 0.0f, 100.0f);

		gs_set_cull_mode(GS_NEITHER);

		float y = font.GetHeight();
		//float height = font.GetHeight() * lines.size();
		//if (height < tex_height)
		//	y = (tex_height - height + font.GetHeight()) / 2.0f;

		gs_texrender_reset(drawTexRender);
		if (gs_texrender_begin(drawTexRender, tex_width, tex_height)) {

			// clear
			vec4 black;
			vec4_zero(&black);
			gs_clear(GS_CLEAR_COLOR | GS_CLEAR_DEPTH, &black, 1.0f, 0);

			for (int i = 0; i < lines.size(); i++) {
				float x = (tex_width - font.GetTextWidth(lines[i])) / 2;
				font.RenderText(lines[i], x, y);
				y += font.GetHeight();
			}

			gs_texrender_end(drawTexRender);
		}
		gs_matrix_pop();
		gs_viewport_pop();
		gs_projection_pop();

		return gs_texrender_get_texture(drawTexRender);
	}



} // smll namespace

