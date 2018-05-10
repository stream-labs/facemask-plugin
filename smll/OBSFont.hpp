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
#pragma once

extern "C" {
#pragma warning( push )
#pragma warning( disable: 4201 )
#include <libobs/graphics/graphics.h>
#include <libobs/graphics/vec2.h>
#pragma warning( pop )
}

#include <vector>
#include <string>
#include "smll/OBSTexture.hpp"

namespace smll {

	class OBSFont
	{
	public:

		OBSFont(const std::string& filename="c:/Windows/Fonts/Arial.ttf", int size=48);
		~OBSFont();

		void RenderText(const std::string& text, float x, float y);
		float GetTextWidth(const std::string& text) const;
		int GetSize() const { return m_size; }
		int GetHeight() const { return m_height; }

		int CountNumLines(const std::string& text, int max_width) const;
		std::vector<std::string> BreakIntoLines(const std::string& text, int max_width) const;

	private:
		class FontInfo {
		public:
			vec2			pos;
			vec2			size;
			vec2			bearing;
			float			advance;
		};

		gs_effect_t*			m_effect;
		OBSTexture				m_texture;
		gs_vb_data*				m_vertexData;
		gs_vertbuffer_t*		m_vertexBuffer;
		std::vector<FontInfo>	m_fontInfos;
		int						m_size;
		int						m_height;

		void SetFont(const std::string& filename, int size);
		void DestroyFontInfo();
		void UpdateAndDrawVertices(float w, float h, 
			float u1, float u2, float v1, float v2);
	};

}

