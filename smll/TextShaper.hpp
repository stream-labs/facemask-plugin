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
#include "smll/OBSFont.hpp"

namespace smll {

	class TextShaper
	{
	public:

		TextShaper(const std::wstring& s=L"");
		~TextShaper();

		void SetString(const std::wstring& s);
		const std::wstring& GetString() const { return m_text; }

		int GetOptimalSize(OBSFont& font, 
			int target_width, int target_height);
		std::vector<std::wstring> GetLines(OBSFont& font, 
			int size, int target_width);

	private:

		struct Word {
			int		index;
			int		length;
			Word() : index(0), length(0) {}
		};

		std::wstring			m_text;
		std::vector<Word>	m_words;

		bool WillTextFit(OBSFont& font, int size,
			int target_width, int target_height);
		float GetWordWidth(OBSFont& font, int size,
			const Word& word);
		void GetActualWords(OBSFont& font, int size, 
			float line_limit, std::vector<Word>& words);
	};

}

