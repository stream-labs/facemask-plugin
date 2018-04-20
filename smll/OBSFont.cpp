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

#pragma warning( pop )

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H


namespace smll {

	OBSFont::OBSFont(const std::string& filename, int size) {
		SetFont(filename, size);
	}

	OBSFont::~OBSFont() {
	}

	void OBSFont::SetFont(const std::string& filename, int size) {

		// Init FreeType
		FT_Library ft;
		if (FT_Init_FreeType(&ft))
			blog(LOG_ERROR, "ERROR::FREETYPE: Could not init FreeType Library");

		// Load font as face
		FT_Face face;
		if (FT_New_Face(ft, filename.c_str(), 0, &face))
			blog(LOG_ERROR, "ERROR::FREETYPE: Failed to load font");

		// Set size to load glyphs as
		FT_Set_Pixel_Sizes(face, 0, size);

		obs_enter_graphics();

		// Let's do ascii 32 - 126
		m_fontInfos.clear();
		for (char c = 32; c < 127; c++)
		{
			// Load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				blog(LOG_ERROR, "ERROR::FREETYTPE: Failed to load Glyph");
				continue;
			}

			FontInfo fi;
			vec2_set(&(fi.size), (float)face->glyph->bitmap.width, (float)face->glyph->bitmap.rows);
			vec2_set(&(fi.bearing), (float)face->glyph->bitmap_left, (float)-face->glyph->bitmap_top);
			fi.advance = (float)face->glyph->advance.x;
			fi.texture = gs_texture_create(face->glyph->bitmap.width, 
				face->glyph->bitmap.rows, GS_R8, 1, 
				(const uint8_t**)&(face->glyph->bitmap.buffer), 0);

			m_fontInfos.emplace_back(fi);
		}

		obs_leave_graphics();

		// Destroy FreeType once we're finished
		FT_Done_Face(face);
		FT_Done_FreeType(ft);
	}

	void OBSFont::RenderText(const std::string& text, float xx, float y) {
		gs_effect_t* defaultEffect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

		std::string::const_iterator c;
		float x = xx;
		for (c = text.begin(); c != text.end(); c++) {
			int idx = (int)*c - 32;
			FontInfo& fi = m_fontInfos[idx];

			gs_matrix_push();
			gs_matrix_translate3f(xx + fi.bearing.x, y + fi.bearing.y, 0.0f);
			while (gs_effect_loop(defaultEffect, "Draw")) {
				gs_effect_set_texture(gs_effect_get_param_by_name(defaultEffect,
					"image"), fi.texture);
				gs_draw_sprite(fi.texture, 0, fi.size.x, fi.size.y);
			}
			gs_matrix_pop();
			xx += (fi.advance / 64.0f); // advance is 1/64 pixels
		}
	}

}
