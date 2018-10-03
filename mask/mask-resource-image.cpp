/*
 * Face Masks for SlOBS
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

#include "mask-resource-image.h"
#include "plugin/exceptions.h"
#include "plugin/plugin.h"
#include "plugin/utils.h"
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/util/platform.h>
	#include <libobs/obs-module.h>
	#pragma warning( pop )
}
#include <sstream>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <thread>

static const char* const S_DATA = "data";
static const char* const S_WIDTH = "width";
static const char* const S_HEIGHT = "height";
static const char* const S_MIP_LEVELS = "mip-levels";
static const char* const S_MIP_DATA = "mip-data-%d";
static const char* const S_BPP = "bpp";

static const unsigned int MAX_MIP_LEVELS = 32;


Mask::Resource::Image::Image(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name), m_width(0), m_height(0), m_fmt(GS_RGBA) {

	char mipdat[128];
	snprintf(mipdat, sizeof(mipdat), S_MIP_DATA, 0);

	// Could be PNG data or raw texture data. See which.

	// PNG DATA?
	if (obs_data_has_user_value(data, S_DATA)) {
		const char* base64data = obs_data_get_string(data, S_DATA);
		if (base64data[0] == '\0') {
			PLOG_ERROR("Image '%s' has empty data.", name.c_str());
			throw std::logic_error("Image has empty data.");
		}

		// save for later
		m_tempFile = Utils::Base64ToTempFile(base64data);
	}

	// RAW DATA?
	else if (obs_data_has_user_value(data, mipdat)) {
		// error checks
		if (!obs_data_has_user_value(data, S_WIDTH)) {
			PLOG_ERROR("Image '%s' has no width.", name.c_str());
			throw std::logic_error("Image has no width.");
		}
		if (!obs_data_has_user_value(data, S_HEIGHT)) {
			PLOG_ERROR("Image '%s' has no height.", name.c_str());
			throw std::logic_error("Image has no height.");
		}
		if (!obs_data_has_user_value(data, S_BPP)) {
			PLOG_ERROR("Image '%s' has no bpp.", name.c_str());
			throw std::logic_error("Image has no bpp.");
		}
		if (!obs_data_has_user_value(data, S_MIP_LEVELS)) {
			PLOG_ERROR("Image '%s' has no mip levels.", name.c_str());
			throw std::logic_error("Image has no mip levels.");
		}

		// get image info
		m_width = (int)obs_data_get_int(data, S_WIDTH);
		m_height = (int)obs_data_get_int(data, S_HEIGHT);
		int bpp = (int)obs_data_get_int(data, S_BPP);
		int mipLevels = (int)obs_data_get_int(data, S_MIP_LEVELS);

		// check format
		if (bpp == 1) {
			m_fmt = GS_R8;
		}
		else if (bpp != 4) {
			PLOG_ERROR("BPP of %d is not supported.", bpp);
			throw std::logic_error("Image has unsupported bpp.");
		}

		// load mip datas
		if (mipLevels > MAX_MIP_LEVELS)
			mipLevels = MAX_MIP_LEVELS;

		int w = m_width;
		int h = m_height;

		for (int i = 0; i < mipLevels; i++) {
			snprintf(mipdat, sizeof(mipdat), S_MIP_DATA, i);
			const char* base64data = obs_data_get_string(data, mipdat);
			if (base64data[0] == '\0') {
				PLOG_ERROR("Image '%s' has empty data.", name.c_str());
				throw std::logic_error("Image has empty data.");
			}
			std::vector<uint8_t> decoded;
			base64_decodeZ(base64data, decoded);
			m_decoded_mips.emplace_back(decoded);
			if (decoded.size() != (w * h * bpp)) {
				size_t ds = decoded.size();

				PLOG_ERROR("Image '%s' size doesnt add up. Should be %d but is %d bytes",
					name.c_str(), (w*h*bpp), ds);
				throw std::logic_error("Image size doesnt add up.");
			}
			w /= 2;
			h /= 2;
		}
	}
	else {
		PLOG_ERROR("Image '%s' has no data.", name.c_str());
		throw std::logic_error("Image has no data.");
	}

}

Mask::Resource::Image::Image(Mask::MaskData* parent, std::string name, std::string filename) 
	: IBase(parent, name) {

	m_Texture = std::make_shared<GS::Texture>(filename);
}


Mask::Resource::Image::~Image() {}

Mask::Resource::Type Mask::Resource::Image::GetType() {
	return Mask::Resource::Type::Image;
}

void Mask::Resource::Image::Update(Mask::Part* part, float time) {
	UNUSED_PARAMETER(part);
	UNUSED_PARAMETER(time);
}

void Mask::Resource::Image::Render(Mask::Part* part) {
	UNUSED_PARAMETER(part);
	if (m_Texture == nullptr) {

		// temp file?
		if (m_tempFile.length() > 0) {
			m_Texture = std::make_shared<GS::Texture>(m_tempFile);
			Utils::DeleteTempFile(m_tempFile);
			m_tempFile.clear();
		}
		else {
			int mipLevels = (int)m_decoded_mips.size();
			const uint8_t* mips[MAX_MIP_LEVELS];
			for (int i = 0; i < mipLevels; i++) {
				mips[i] = m_decoded_mips[i].data();
			}
			m_Texture = std::make_shared<GS::Texture>(m_width, m_height, m_fmt, mipLevels, mips, 0);
			m_decoded_mips.clear();
		}
	}
}
