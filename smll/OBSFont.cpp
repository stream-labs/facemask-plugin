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

// From this example:
//
// https://learnopengl.com/In-Practice/Text-Rendering
//
//


#include "OBSFont.hpp"

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
#include "utils.h"
#include <opencv2/opencv.hpp>

#define MAX_TEXTURE_WIDTH	(2048)
#define MAX_TEXTURE_HEIGHT	(4096)
#define MAX_TEXTURE_SIZE	(MAX_TEXTURE_WIDTH * MAX_TEXTURE_HEIGHT)

#pragma warning( pop )

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H


namespace smll {

	OBSFont::OBSFont(const std::string& filename, int minSize, int maxSize) {

		// make our own sprite vertex buffer
		m_vertexData = gs_vbdata_create();
		m_vertexData->num = 4;
		m_vertexData->points = (vec3*)bmalloc(sizeof(struct vec3) * 4);
		m_vertexData->num_tex = 1;
		m_vertexData->tvarray = (gs_tvertarray*)bmalloc(sizeof(struct gs_tvertarray));
		m_vertexData->tvarray[0].width = 2;
		m_vertexData->tvarray[0].array = bmalloc(sizeof(struct vec2) * 4);
		memset(m_vertexData->points, 0, sizeof(struct vec3) * 4);
		memset(m_vertexData->tvarray[0].array, 0, sizeof(struct vec2) * 4);

		// GS stuff
		obs_enter_graphics();
		char* f = obs_module_file("effects/color_alpha_tex.effect");
		char* errorMessage = nullptr;
		m_effect = gs_effect_create_from_file(f, &errorMessage);
		if (!m_effect || errorMessage) {
			std::string error(errorMessage);
			bfree((void*)errorMessage);
			obs_leave_graphics();
			throw std::runtime_error(error);
		}
		bfree(f);
		m_vertexBuffer = gs_vertexbuffer_create(m_vertexData, GS_DYNAMIC);
		f = obs_module_file("resources/kevin.png");
		m_kevin.texture = gs_texture_create_from_file(f);
		bfree(f);
		m_kevin.width = gs_texture_get_width(m_kevin.texture);
		m_kevin.height = gs_texture_get_height(m_kevin.texture);
		obs_leave_graphics();

		// No bitmap font rendered
		m_texture.width = 0;
		m_texture.height = 0;
		m_texture.texture = nullptr;

		SetFont(filename, minSize, maxSize);
	}

	OBSFont::~OBSFont() {
		DestroyFontInfo();

		obs_enter_graphics();
		gs_effect_destroy(m_effect);
		gs_vertexbuffer_destroy(m_vertexBuffer);
		gs_vbdata_destroy(m_vertexData);
		gs_texture_destroy(m_kevin.texture);
		obs_leave_graphics();
	}

	void OBSFont::DestroyFontInfo() {
		obs_enter_graphics();
		if (m_texture.texture)
			gs_texture_destroy(m_texture.texture);
		obs_leave_graphics();
		m_texture.width = 0;
		m_texture.height = 0;
		m_texture.texture = nullptr;
		m_fontInfos.clear();
	}

	void OBSFont::SetFont(const std::string& filename, int minSize, int maxSize) {
		if (m_filename != filename) {
			// init file info
			m_filename = filename;
			m_minSize = minSize;
			m_maxSize = maxSize;
			m_advances.clear();

			// Init FreeType
			FT_Library ft;
			if (FT_Init_FreeType(&ft)) {
				blog(LOG_ERROR, "ERROR::FREETYPE: Could not init FreeType Library");
				return;
			}

			// Load font as face
			FT_Face face;
			if (FT_New_Face(ft, m_filename.c_str(), 0, &face)) {
				blog(LOG_ERROR, "ERROR::FREETYPE: Failed to load font");
				return;
			}

			blog(LOG_DEBUG, "Font Family: %s  Style: %s",
				face->family_name, face->style_name);

			for (int size = minSize; size <= maxSize; size++) {
				std::array<float, NUM_CHARACTERS> advances;

				// Set size to load glyphs as
				FT_Set_Pixel_Sizes(face, 0, size);

				float height = 0.0f;
				for (char c = LOWEST_CHARACTER; c <= HIGHEST_CHARACTER; c++)
				{
					// Load character glyph 
					if (FT_Load_Char(face, c, FT_LOAD_DEFAULT))
					{
						blog(LOG_ERROR, "ERROR::FREETYTPE: Failed to load Glyph");
						continue;
					}

					int index = (int)c - (int)LOWEST_CHARACTER;
					advances[index] = (float)face->glyph->advance.x / 64.0f; // 26.6 format

					float  glyph_height = (float)face->glyph->metrics.height / 64.0f;
					if (glyph_height > height)
						height = glyph_height;
				}

				m_heights.emplace_back(height);
				m_advances.emplace_back(advances);
			}	
		}
	}

	void OBSFont::RenderBitmapFont(int size) {

		// sanity check
		if (m_filename.size() < 1)
			return;
		if (size == m_size)
			return;

		// save the size
		m_size = size;

		// Init FreeType
		FT_Library ft;
		if (FT_Init_FreeType(&ft)) {
			blog(LOG_ERROR, "ERROR::FREETYPE: Could not init FreeType Library");
			return;
		}

		// Load font as face
		FT_Face face;
		if (FT_New_Face(ft, m_filename.c_str(), 0, &face)) {
			blog(LOG_ERROR, "ERROR::FREETYPE: Failed to load font");
			return;
		}

		// Set size to load glyphs as
		FT_Set_Pixel_Sizes(face, 0, size);

		DestroyFontInfo();

		// temp buffer for texture data
		char* textureData = new char[MAX_TEXTURE_SIZE];
		cv::Mat dstMat = cv::Mat(MAX_TEXTURE_HEIGHT, MAX_TEXTURE_WIDTH, CV_8UC1,
			textureData);

		// figure a reasonable width
		int reasonable_width = size * 10;

		// Let's do ascii 32 - 126
		m_height = 0;
		int actual_width = 0;
		int row_height = 0;
		int x = 0;
		int y = 1;
		for (char c = LOWEST_CHARACTER; c <= HIGHEST_CHARACTER; c++)
		{
			// Load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				blog(LOG_ERROR, "ERROR::FREETYTPE: Failed to load Glyph");
				continue;
			}

			// Maximum heights
			if (face->glyph->bitmap.rows > m_height)
				m_height = face->glyph->bitmap.rows;
			if (face->glyph->bitmap.rows > row_height)
				row_height = face->glyph->bitmap.rows;

			// End of line?
			if ((x + face->glyph->bitmap.width) > reasonable_width) {
				if (x > actual_width)
					actual_width = x;
				x = 0;
				y += row_height + 1;
				row_height = 0;
			}

			// Create font info
			FontInfo fi;
			vec2_set(&(fi.size), (float)face->glyph->bitmap.width, (float)face->glyph->bitmap.rows);
			vec2_set(&(fi.bearing), (float)face->glyph->bitmap_left, (float)-face->glyph->bitmap_top);
			fi.advance = (float)face->glyph->advance.x / 64.0f; // advance is 1/64 pixels

			// Copy bitmap 
			if (face->glyph->bitmap.rows > 0 && face->glyph->bitmap.width > 0) {
				cv::Mat srcMat = cv::Mat(face->glyph->bitmap.rows, face->glyph->bitmap.width, CV_8UC1,
					face->glyph->bitmap.buffer, face->glyph->bitmap.pitch);
				srcMat.copyTo(dstMat.rowRange(y, y + face->glyph->bitmap.rows).colRange(x, x + face->glyph->bitmap.width));
				vec2_set(&(fi.pos), (float)x, (float)y);
			}
			else {
				vec2_set(&(fi.pos), (float)-1, (float)-1);
			}

			x += face->glyph->bitmap.width + 1;
			m_fontInfos.emplace_back(fi);
		}

		int actual_height = y + row_height + 1;

		// Create actual texture data
		actual_width = (int)ALIGN_16(actual_width);
		actual_height = (int)ALIGN_16(actual_height);
		char* actualTextureData = new char[actual_width * actual_height];
		cv::Mat actualMat = cv::Mat(actual_height, actual_width, CV_8UC1,
			actualTextureData);
		dstMat.rowRange(0, actual_height).colRange(0, actual_width).copyTo(actualMat);

		// DEBUG: write out image
		//char temp[256];
		//snprintf(temp, sizeof(temp), "c:/temp/font-%d.png", size);
		//cv::imwrite(temp, actualMat);

		// Create the texture
		obs_enter_graphics();
		m_texture.width = actual_width;
		m_texture.height = actual_height;
		m_texture.texture = gs_texture_create(actual_width, actual_height, GS_R8, 1,
			(const uint8_t**)&actualTextureData, 0);
		obs_leave_graphics();
		delete[] textureData;
		delete[] actualTextureData;

		// Destroy FreeType once we're finished
		FT_Done_Face(face);
		FT_Done_FreeType(ft);
	}

	void OBSFont::RenderText(const std::string& text, float xx, float y) {

		vec4 color;
		vec4_set(&color, 0.0f, 0.0f, 0.0f, 1.0f);

		gs_enable_blending(true);
		gs_blend_function(gs_blend_type::GS_BLEND_SRCALPHA,
			gs_blend_type::GS_BLEND_INVSRCALPHA);

		gs_effect_t* defaultEffect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++) {
			int idx = (int)*c - LOWEST_CHARACTER;
			if (idx < 0 || idx >= m_fontInfos.size()) {
				gs_matrix_push();
				gs_matrix_translate3f(xx, y - ((float)m_height * 0.84f), 0.0f);
				while (gs_effect_loop(defaultEffect, "Draw")) {
					gs_effect_set_texture(gs_effect_get_param_by_name(defaultEffect,
						"image"), m_kevin.texture);
					gs_draw_sprite(m_kevin.texture, 0, m_height - 2, m_height - 2);
					xx += m_height;
				}
				gs_matrix_pop();
				continue;
			}
			const FontInfo& fi = m_fontInfos[idx];

			if (fi.size.x > 0 && fi.size.y > 0) {
				float u1 = fi.pos.x / (float)(m_texture.width - 1);
				float u2 = (fi.pos.x + fi.size.x) / (float)(m_texture.width - 1);
				float v1 = fi.pos.y / (float)(m_texture.height - 1);
				float v2 = (fi.pos.y + fi.size.y) / (float)(m_texture.height - 1);

				gs_matrix_push();
				gs_matrix_translate3f(xx + fi.bearing.x, y + fi.bearing.y, 0.0f);
				while (gs_effect_loop(m_effect, "Draw")) {
					gs_effect_set_vec4(gs_effect_get_param_by_name(m_effect,
						"color"), &color);
					gs_effect_set_texture(gs_effect_get_param_by_name(m_effect,
						"image"), m_texture.texture);
					UpdateAndDrawVertices((float)fi.size.x, (float)fi.size.y, u1, u2, v1, v2);
				}
				gs_matrix_pop();
			}
			xx += fi.advance;
		}
	}

	float OBSFont::GetTextWidth(int size, const std::string& text) const {
		float w = 0;
		std::string::const_iterator c;
		int aidx = size - m_minSize;
		if (aidx < 0 || aidx >= m_advances.size())
			return 0.0f;
		const std::array<float, NUM_CHARACTERS>& advances = m_advances[aidx];
		bool lastkevin = false;
		for (c = text.begin(); c != text.end(); c++) {
			int idx = (int)*c - LOWEST_CHARACTER;
			if (idx < 0 || idx >= NUM_CHARACTERS)
				w += m_heights[aidx]; // kevin
			else
				w += advances[idx];
		}
		return w;
	}

	float OBSFont::GetFontHeight(int size) const {
		return m_heights[size - m_minSize];
	}

	float OBSFont::GetCharAdvance(int size, char c) const {
		int aidx = size - m_minSize;
		int cidx = (int)c - LOWEST_CHARACTER;
		if (aidx < 0 || aidx >= m_advances.size())
			return 0;
		if (cidx < 0 || cidx >= NUM_CHARACTERS)
			return m_heights[aidx]; // kevin
		return m_advances[aidx][cidx];
	}

	void OBSFont::UpdateAndDrawVertices(float w, float h, 
		float u1, float u2, float v1, float v2) {
		vec3_set(&(m_vertexData->points[0]), 0.0f, 0.0f, 0.0f);
		vec3_set(&(m_vertexData->points[1]), w, 0.0f, 0.0f);
		vec3_set(&(m_vertexData->points[2]), 0.0f, h, 0.0f);
		vec3_set(&(m_vertexData->points[3]), w, h, 0.0f);

		vec2* uvs = (vec2*)m_vertexData->tvarray[0].array;
		vec2_set(&(uvs[0]), u1, v1);
		vec2_set(&(uvs[1]), u2, v1);
		vec2_set(&(uvs[2]), u1, v2);
		vec2_set(&(uvs[3]), u2, v2);

		gs_vertexbuffer_flush(m_vertexBuffer);
		gs_load_vertexbuffer(m_vertexBuffer);
		gs_load_indexbuffer(NULL);
		gs_draw(GS_TRISTRIP, 0, 0);
	}

}
