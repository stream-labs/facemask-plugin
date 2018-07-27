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

#include <map>
#include <vector>
#include <array>
#include <string>
#include "smll/OBSTexture.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace smll {

	class OBSFont
	{
	public:

		OBSFont(const std::string& filename="c:/Windows/Fonts/Arial.ttf", int minSize=16, int maxSize=250);
		~OBSFont();

		// Render out the font at given size
		void RenderBitmapFont(int size);

		// For the currently rendered font
		void RenderText(const std::wstring& wtext, float x, float y);
		int GetSize() const { return m_size; }
		int GetHeight();

		// Font metrics
		int GetMinSize() const { return m_minSize; }
		int GetMaxSize() const { return m_maxSize; }
		float GetFontHeight(int size) const;
		float GetCharAdvance(int size, FT_ULong c);
		float GetTextWidth(int size, const std::wstring& text);
		void  UpdateFontInfo(const std::wstring& text, int size);
		void  Reset();

	private:
		FT_Face face;
		bool inited = false;
		class FontInfo {
		public:
			vec2			pos;
			vec2			size;
			vec2			bearing;
			float			advance;
			//texture
			OBSTexture		texture;
		};

		// the font
		std::string				                  m_filename;
		int						                  m_minSize;
		int						                  m_maxSize;
		std::map<std::pair<FT_ULong,int>, float>  m_advances;
		std::map<int, float>		              m_heights;
		std::map<int, float>		              m_gl_heights;
		// rendering
		gs_effect_t*			m_effect;
		gs_vb_data*				m_vertexData;
		gs_vertbuffer_t*		m_vertexBuffer;
		OBSTexture				m_kevin;

		// rendered bitmap font data
		std::map<std::pair<FT_ULong, int>, FontInfo>	m_fontInfos;
		int	m_size   = 0;


		void SetFont(const std::string& filename, int minSize = 16, int maxSize = 300);
		void DestroyFontInfo();
		void UpdateAndDrawVertices(float w, float h, 
			float u1, float u2, float v1, float v2);
	};

}

