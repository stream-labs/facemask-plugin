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

namespace smll {

	class TextShaper
	{
	public:

		TextShaper(const std::string& s);
		~TextShaper();

		void SetString(const std::string& s);

	private:

		struct Word {
			int		index;
			int		length;
			Word() : index(0), length(0) {}
		};

		std::string			m_text;
		std::vector<Word>	m_words;
	};

}

