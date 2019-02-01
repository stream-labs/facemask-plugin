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

static const unsigned int MAX_MIP_LEVELS = 32;

Mask::Resource::Image::Image(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name), m_width(0), m_height(0), m_fmt(GS_RGBA), m_mipLevels(-1), m_is_cubemap(false) {

	char mipdat[128];
	snprintf(mipdat, sizeof(mipdat), S_MIP_DATA, 0);

	char side_mipdat[128];
	snprintf(side_mipdat, sizeof(side_mipdat), S_SIDE_MIP_DATA, 0, 0);

	if (obs_data_has_user_value(data, S_CHANNEL_FORMAT)) {

		const char* ch = obs_data_get_string(data, S_CHANNEL_FORMAT);
		std::string ch_fmt_str(ch);
		if (ch_fmt_str == "float32") {
			m_fmt = GS_RGBA32F;
		}
		// else ch_fmt_str == "int8" -> default = GS_RGBA
	}

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
		m_mipLevels = (int)obs_data_get_int(data, S_MIP_LEVELS);

		// check format
		if (bpp == 1) {
			m_fmt = GS_R8;
		}
		else if (bpp != 4) {
			PLOG_ERROR("BPP of %d is not supported.", bpp);
			throw std::logic_error("Image has unsupported bpp.");
		}

		// load mip datas
		if (m_mipLevels > MAX_MIP_LEVELS)
			m_mipLevels = MAX_MIP_LEVELS;

		int w = m_width;
		int h = m_height;

		for (int i = 0; i < m_mipLevels; i++) {
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
	else if (obs_data_has_user_value(data, side_mipdat)) {
		m_is_cubemap = true;
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
		m_mipLevels = (int)obs_data_get_int(data, S_MIP_LEVELS);

		// check format
		if (bpp == 1) {
			m_fmt = GS_R8;
		}
		else if (bpp != 4) {
			PLOG_ERROR("BPP of %d is not supported.", bpp);
			throw std::logic_error("Image has unsupported bpp.");
		}

		// load mip datas
		if (m_mipLevels > MAX_MIP_LEVELS)
			m_mipLevels = MAX_MIP_LEVELS;

		int w, h;
		size_t fmt_size;
		if (m_fmt == GS_RGBA)
			fmt_size = sizeof(uint8_t);
		else if (m_fmt == GS_RGBA32F)
			fmt_size = sizeof(float);
		else if (m_fmt == GS_R8)
			fmt_size = sizeof(uint8_t);
		else {
			PLOG_ERROR("Unrecognized image format: %d", m_fmt);
			throw std::logic_error("Unrecognized image format.");
		}

		for (size_t side = 0; side < 6; side++)
		{
			w = m_width;
			h = m_height;
			for (int i = 0; i < m_mipLevels; i++) {
				snprintf(side_mipdat, sizeof(side_mipdat), S_SIDE_MIP_DATA, side, i);
				const char* base64data = obs_data_get_string(data, side_mipdat);
				if (base64data[0] == '\0') {
					PLOG_ERROR("Image '%s' has empty data.", name.c_str());
					throw std::logic_error("Image has empty data.");
				}
				std::vector<uint8_t> decoded;
				base64_decodeZ(base64data, decoded);
				m_decoded_mips.emplace_back(decoded);
				if (decoded.size() != (w * h * bpp * fmt_size)) {
					size_t ds = decoded.size();

					PLOG_ERROR("Image '%s' size doesnt add up. Should be %d but is %d bytes",
						name.c_str(), (w*h*bpp), ds);
					throw std::logic_error("Image size doesnt add up.");
				}
				w /= 2;
				h /= 2;
			}
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
			if (m_is_cubemap) {
				const uint8_t* sides_mips[MAX_MIP_LEVELS*6];
				for (size_t side = 0; side < 6; side++)
				{
					for (int i = 0; i < m_mipLevels; i++) {
						sides_mips[side * m_mipLevels + i] = m_decoded_mips[side * m_mipLevels + i].data();
					}
				}
				m_Texture = std::make_shared<GS::Texture>(m_width, m_fmt, m_mipLevels, sides_mips, 0);
				m_decoded_mips.clear();
			}
			else {
				const uint8_t* mips[MAX_MIP_LEVELS];
				for (int i = 0; i < m_mipLevels; i++) {
					mips[i] = m_decoded_mips[i].data();
				}
				m_Texture = std::make_shared<GS::Texture>(m_width, m_height, m_fmt, m_mipLevels, mips, 0);
				m_decoded_mips.clear();
			}
		}
	}
}
