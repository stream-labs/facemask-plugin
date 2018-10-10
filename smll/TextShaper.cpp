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
#include "plugin/utils.h"
#include <opencv2/opencv.hpp>

#define WORD_SPLIT_SIZE_LIMIT	 (20)
#define LINE_LIMIT(W)			 ((float)((W) - 5))

#pragma warning( pop )

namespace smll {

	TextShaper::TextShaper(const std::wstring& s) {
		this->SetString(s);
	}

	TextShaper::~TextShaper() {
	}

	void TextShaper::SetString(const std::wstring& s) {		
		m_words.clear();
		m_text = s;
		Word word;
		bool word_saved = true;
		int idx = 0;
		std::wstring::const_iterator p;
		for (p = m_text.begin(); p != m_text.end(); *p, p++, idx++) {
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

	int TextShaper::GetOptimalSize(OBSFont& font, 
		int target_width, int target_height) {
		//Binary Search
		int min = font.GetMinSize();
		int max = font.GetMaxSize();
		int last_fit = min;
		while(true){
			int size = (max + min) / 2;
			if (min + 1 >= max) {
				break;
			}
			if (WillTextFit(font, size, target_width, target_height)) {
				min = size;
				last_fit = size;
			}
			else {
				max = size;
			}
		}

		return last_fit;;
	}

	std::vector<std::wstring> TextShaper::GetLines(OBSFont& font, 
		int size, int target_width) {

		float line_limit = LINE_LIMIT(target_width);
		std::vector<Word>	words;
		GetActualWords(font, size, line_limit, words);

		std::vector<std::wstring> lines;

		float x = 0.0f;
		float space_width = font.GetCharAdvance(size, ' ');
		std::wstring line = L"";
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
					line += L" "; 
				line += m_text.substr(word.index, word.length);
				x += d;
			}
		}
		if (line.length() > 0)
			lines.push_back(line);

		return lines;
	}

	bool TextShaper::WillTextFit(OBSFont& font, int size,
		int target_width, int target_height) {

		float line_limit = LINE_LIMIT(target_width);
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

	void TextShaper::GetActualWords(OBSFont& font, int size, 
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


	float TextShaper::GetWordWidth(OBSFont& font, int size,
		const Word& word) {
		int end = word.index + word.length;
		float len = 0.0f;
		for (int i = word.index; i < end; i++) {
			len += font.GetCharAdvance(size, m_text[i]);
		}
		return len;
	}

}
