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

#define WORD_SPLIT_SIZE_LIMIT	 (20)

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
	}

	int TextShaper::GetOptimalSize(const OBSFont& font, 
		int target_width, int target_height) {

		for (int size = font.GetMaxSize(); size >= font.GetMinSize(); size--) {
			if (WillTextFit(font, size, target_width, target_height)) {
				return size;
			}
		}

		return font.GetMinSize();
	}

	std::vector<std::string> TextShaper::GetLines(const OBSFont& font, 
		int size, int target_width) {

		float line_limit = (float)(target_width - 1);
		std::vector<Word>	words;
		GetActualWords(font, size, line_limit, words);

		std::vector<std::string> lines;

		float x = 0.0f;
		float space_width = font.GetCharAdvance(size, ' ');
		std::string line = "";
		for (int i = 0; i < words.size(); i++) {
			const Word& word = words[i];
			float w = GetWordWidth(font, size, word);
			float d = w;
			if (line.length() > 0) {
				d += space_width;
			}
			if ((x + d) > line_limit) {
				lines.push_back(line);
				// new line
				line = m_text.substr(word.index, word.length);
				x = w;
			}
			else {
				if (line.length() > 0)
					line += " "; 
				line += m_text.substr(word.index, word.length);
				x += d;
			}
		}
		if (line.length() > 0)
			lines.push_back(line);

		return lines;
	}

	bool TextShaper::WillTextFit(const OBSFont& font, int size,
		int target_width, int target_height) {

		float line_limit = (float)(target_width - 1);
		std::vector<Word>	words;
		GetActualWords(font, size, line_limit, words);

		// see if all the words fit
		float height = font.GetFontHeight(size);
		int max_lines = (int)((float)target_height / height);
		float x = 0.0f;
		int line = 0;
		float space_width = font.GetCharAdvance(size, ' ');
		for (int i = 0; i < words.size(); i++) {
			const Word& word = words[i];
			float w = GetWordWidth(font, size, word);
			if (w > line_limit)
				return false; // word will never fit
			if ((x + space_width + w) > line_limit) {
				line++;
				if (line >= max_lines) {
					return false;
				}
				x = w;
			}
			else {
				x += space_width + w;
			}
		}
		if (x > 0.0f)
			line++;
		if (line >= max_lines) {
			return false;
		}

		return true;
	}

	void TextShaper::GetActualWords(const OBSFont& font, int size, 
		float line_limit, std::vector<Word>& words) {

		// build our list of words
		for (int i = 0; i < m_words.size(); i++) {
			words.emplace_back(m_words[i]);
		}
		// slit words too long to fit on a line
		for (int i = 0; i < words.size(); i++) {
			Word& word = words[i];
			float w = GetWordWidth(font, size, word);
			if (w > line_limit && word.length >= WORD_SPLIT_SIZE_LIMIT) {
				// split
				Word new_word;
				int ll = word.length / 2;
				new_word.index = word.index + ll;
				new_word.length = word.length - ll;
				word.length = ll;
				words.insert(words.begin() + i + 1, new_word);
				i--;
			}
		}
	}


	float TextShaper::GetWordWidth(const OBSFont& font, int size,
		const Word& word) {
		int end = word.index + word.length;
		float len = 0.0f;
		for (int i = word.index; i < end; i++) {
			len += font.GetCharAdvance(size, m_text[i]);
		}
		return len;
	}

}
