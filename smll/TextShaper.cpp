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
#include "TextShaper.hpp"

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
#include "utils.h"
#include <opencv2/opencv.hpp>

#pragma warning( pop )

namespace smll {

	TextShaper::TextShaper(const std::string& s) {
		this->SetString(s);
	}

	TextShaper::~TextShaper() {
	}

	void TextShaper::SetString(const std::string& s) {
		m_text = s;
		m_words.clear();
		Word word;
		bool word_saved = true;
		int idx = 0;
		for (const char* p = m_text.data(); *p; p++, idx++) {
			if (*p == ' ') {
				if (!word_saved) {
					m_words.push_back(word);
					word_saved = true;
				}
			}
			else {
				if (word_saved) {
					word.index = idx;
					word.length = 0;
					word_saved = false;
				}
				word.length++;
			}
		}
		if (!word_saved) {
			m_words.push_back(word);
			word_saved = true;
		}

		blog(LOG_DEBUG, "      012345678901234567890123456789012345678901234567890");
		blog(LOG_DEBUG, "text: %s", s.c_str());
		for (int i = 0; i < m_words.size(); i++) {
			blog(LOG_DEBUG, "words[%d]: %d  length: %d", i, m_words[i].index, m_words[i].length);
		}
	}
}
