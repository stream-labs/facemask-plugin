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

#pragma warning( pop )

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H


namespace smll {

	OBSFont::OBSFont(const std::string& filename, const std::string& base_filename, int minSize, int maxSize) {

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

		SetFont(filename, base_filename, minSize, maxSize);
	}

	OBSFont::~OBSFont() {
		DestroyFontInfo();

		obs_enter_graphics();
		gs_effect_destroy(m_effect);
		gs_vertexbuffer_destroy(m_vertexBuffer);
		//nope gs_vbdata_destroy(m_vertexData);
		gs_texture_destroy(m_kevin.texture);
		obs_leave_graphics();
	}

	void OBSFont::Reset() {
		m_advances.clear();
		m_heights.clear();
		m_gl_heights.clear();
		DestroyFontInfo();
	}

	void OBSFont::DestroyFontInfo() {
		obs_enter_graphics();
		obs_leave_graphics();

		std::map <std::pair< FT_ULong, int > , FontInfo > ::iterator it;
		for (it = m_fontInfos.begin(); it != m_fontInfos.end(); ++it)
			gs_texture_destroy(it->second.texture.texture);
		m_fontInfos.clear();
	}

	void OBSFont::SetFont(const std::string& filename, const std::string& base_filename, int minSize, int maxSize) {
		if (m_filename != filename && m_base_filename != base_filename) {
			// init file info
			if (inited) {
				FT_Done_Face(face);
				FT_Done_Face(base_face);
			}
			inited = false;
			m_filename = filename;
			m_base_filename = base_filename;
			m_minSize = minSize;
			m_maxSize = maxSize;
			m_advances.clear();
			m_fontInfos.clear();
			m_heights.clear();
			m_gl_heights.clear();
			// Init FreeType
			FT_Library ft, base_ft;
			if (FT_Init_FreeType(&ft) || FT_Init_FreeType(&base_ft)) {
				blog(LOG_ERROR, "ERROR::FREETYPE: Could not init FreeType Library");
				return;
			}

			// Load font as face
			if (FT_New_Face(ft, m_filename.c_str(), 0, &face)) {
				blog(LOG_ERROR, "ERROR::FREETYPE: Failed to load font %s", m_filename.c_str());
				return;
			}

			// Load base font as face
			if (FT_New_Face(base_ft, m_base_filename.c_str(), 0, &base_face)) {
				FT_Done_Face(face);
				blog(LOG_ERROR, "ERROR::FREETYPE: Failed to load base font %s", m_base_filename.c_str());
				return;
			}

			inited = true;

			blog(LOG_DEBUG, "Font Family: %s  Style: %s",
				face->family_name, face->style_name);
			blog(LOG_DEBUG, "Base Font Family: %s  Style: %s",
				base_face->family_name, base_face->style_name);
		}
	}

	void OBSFont::UpdateFontInfo(const std::wstring &text, int size) {
		if (!inited) {
			return;
		}
		bool do_render = size == m_size;
		FT_Set_Pixel_Sizes(face, 0, size);
		FT_Set_Pixel_Sizes(base_face, 0, size);
		std::wstring::const_iterator c;
		for (c = text.begin(); c != text.end(); c++) {
			FT_ULong character = *c;
			FT_UInt index;
			FT_Face curr_face = face;
			std::pair<FT_ULong, int> charPair(character, size);

			if ((!do_render && m_advances.find(charPair) != m_advances.end()) ||m_fontInfos.find(charPair) != m_fontInfos.end()) { //already in a map
				continue;
			}
			int index_char = FT_Get_Char_Index(curr_face, character);
			if (index_char == 0) {
				curr_face = base_face;
				index_char = FT_Get_Char_Index(curr_face, character);
			}
			if (FT_Load_Char(curr_face, character, FT_LOAD_RENDER))
			{
				blog(LOG_ERROR, "ERROR::FREETYTPE: Failed to load Glyph");
				continue;
			}

			m_advances[charPair] = (float)curr_face->glyph->advance.x / 64.0f; // 26.6 format

			float  glyph_height = (float)curr_face->glyph->metrics.height / 64.0f;
			float  height = m_heights[size];
			if (m_heights.find(size) == m_heights.end() ||  glyph_height > m_heights[size])
				m_heights[size] = glyph_height;
			if (curr_face->glyph->bitmap.rows > m_gl_heights[size])
				m_gl_heights[size] = curr_face->glyph->bitmap.rows;

			// figure a reasonable width
			int reasonable_width = size * 10;
			int actual_width = 0;
			int row_height = 0;
			int x = 0;
			int y = 1;
			if (do_render && index_char != 0) {
				FontInfo fi;
				vec2_set(&(fi.size), (float)curr_face->glyph->bitmap.width, (float)curr_face->glyph->bitmap.rows);
				vec2_set(&(fi.bearing), (float)curr_face->glyph->bitmap_left, (float)-curr_face->glyph->bitmap_top);
				fi.advance = (float)curr_face->glyph->advance.x / 64.0f; // advance is 1/64 pixels
				vec2_set(&(fi.pos), (float)0, (float)0);

				obs_enter_graphics();
				fi.texture.width = curr_face->glyph->bitmap.width;
				fi.texture.height = curr_face->glyph->bitmap.rows;
				fi.texture.texture = gs_texture_create(curr_face->glyph->bitmap.width, curr_face->glyph->bitmap.rows, GS_R8, 1,
					(const uint8_t**)&curr_face->glyph->bitmap.buffer, 0);
				obs_leave_graphics();

				m_fontInfos[charPair] = fi;
			}
		}
	}

	void OBSFont::RenderBitmapFont(int size) {
		Reset();
		m_size = size;
	}

	void OBSFont::RenderText(const std::wstring& wtext, float xx, float y) {
		UpdateFontInfo(wtext, m_size);
		vec4 color;
		vec4_set(&color, 0.0f, 0.0f, 0.0f, 1.0f);

		gs_enable_blending(true);
		gs_blend_function(gs_blend_type::GS_BLEND_SRCALPHA,
			gs_blend_type::GS_BLEND_INVSRCALPHA);

		gs_effect_t* defaultEffect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

		float m_height = GetHeight();
		std::wstring::const_iterator c;
		for (c = wtext.begin(); c != wtext.end(); c++) {
			FT_ULong idx = *c;
			std::pair<FT_ULong, int> charPair(idx, m_size);
			if ( m_fontInfos.find(charPair) == m_fontInfos.end()) { // no such symbol
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
			const FontInfo& fi = m_fontInfos[charPair];

			if (fi.size.x > 0 && fi.size.y > 0) {
				float u1 = fi.pos.x / (float)(fi.texture.width);
				float u2 = (fi.pos.x + fi.size.x) / (float)(fi.texture.width);
				float v1 = fi.pos.y / (float)(fi.texture.height - 1);
				float v2 = (fi.pos.y + fi.size.y) / (float)(fi.texture.height);

				gs_matrix_push();
				gs_matrix_translate3f(xx + fi.bearing.x, y + fi.bearing.y, 0.0f);
				while (gs_effect_loop(m_effect, "Draw")) {
					gs_effect_set_vec4(gs_effect_get_param_by_name(m_effect,
						"color"), &color);
					gs_effect_set_texture(gs_effect_get_param_by_name(m_effect,
						"image"), fi.texture.texture);
					UpdateAndDrawVertices((float)fi.size.x, (float)fi.size.y, u1, u2, v1, v2);
				}
				gs_matrix_pop();
			}
			xx += fi.advance;
		}
	}

	float OBSFont::GetTextWidth(int size, const std::wstring& text)  {
		UpdateFontInfo(text, size);
		float w = 0;
		std::wstring::const_iterator c;
		for (c = text.begin(); c != text.end(); c++) {
			FT_ULong character = *c;
			std::pair<FT_ULong, int> charPair(character, m_size);
			if (m_fontInfos.find(charPair) == m_fontInfos.end()) {
				if (m_heights.find(size) != m_heights.end()){//kevin
					w += m_heights.at(size);
				}
			}	
			else {
				w += m_advances.at(charPair);
			}
		}
		return w;
	}

	int OBSFont::GetHeight() {
		if (m_gl_heights.find(m_size) != m_gl_heights.end())
			return m_gl_heights.at(m_size);
		return 0;
	}

	float OBSFont::GetFontHeight(int size) const {
		if (m_heights.find(size) != m_heights.end())
			return m_heights.at(size);
		return 0;
	}

	float OBSFont::GetCharAdvance(int size, FT_ULong character) {
		std::wstring text = L"" + character;
		UpdateFontInfo(text, size);
		std::pair<FT_ULong, int> charPair(character, m_size);
		if (m_fontInfos.find(charPair) != m_fontInfos.end())
			return m_advances.at(charPair); 
		return GetFontHeight(size);//kevin
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
