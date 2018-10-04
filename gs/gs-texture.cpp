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

#include "gs-texture.h"


GS::Texture::Texture(uint32_t width, uint32_t height, gs_color_format format, uint32_t mip_levels, const uint8_t **mip_data, uint32_t flags) : m_destroy(true) {
	if (width == 0)
		throw std::logic_error("width must be at least 1");
	if (height == 0)
		throw std::logic_error("height must be at least 1");
	if (mip_levels == 0)
		throw std::logic_error("mip_levels must be at least 1");
	if (!mip_data)
		throw std::logic_error("mip_data is invalid");

	if (mip_levels > 1 || flags & Flags::BuildMipMaps) {
		bool isPOT = (pow(2, (int64_t)floor(log(width) / log(2))) == width)
			&& (pow(2, (int64_t)floor(log(height) / log(2))) == height);
		if (!isPOT)
			throw std::logic_error("mip mapping requires power of two dimensions");
	}

	obs_enter_graphics();
	m_texture = gs_texture_create(width, height, format, mip_levels, mip_data, (flags & Flags::Dynamic) ? GS_DYNAMIC : 0 | (flags & Flags::BuildMipMaps) ? GS_BUILD_MIPMAPS : 0);
	obs_leave_graphics();

	if (!m_texture)
		throw std::runtime_error("Failed to create texture.");
}

GS::Texture::Texture(uint32_t width, uint32_t height, uint32_t depth, gs_color_format format, uint32_t mip_levels, const uint8_t **mip_data, uint32_t flags) : m_destroy(true) {
	if (width == 0)
		throw std::logic_error("width must be at least 1");
	if (height == 0)
		throw std::logic_error("height must be at least 1");
	if (depth == 0)
		throw std::logic_error("depth must be at least 1");
	if (mip_levels == 0)
		throw std::logic_error("mip_levels must be at least 1");
	if (!mip_data)
		throw std::logic_error("mip_data is invalid");

	if (mip_levels > 1 || flags & Flags::BuildMipMaps) {
		bool isPOT = (pow(2, (int64_t)floor(log(width) / log(2))) == width)
			&& (pow(2, (int64_t)floor(log(height) / log(2))) == height)
			&& (pow(2, (int64_t)floor(log(depth) / log(2))) == depth);
		if (!isPOT)
			throw std::logic_error("mip mapping requires power of two dimensions");
	}

	obs_enter_graphics();
	m_texture = gs_voltexture_create(width, height, depth, format, mip_levels, mip_data, (flags & Flags::Dynamic) ? GS_DYNAMIC : 0 | (flags & Flags::BuildMipMaps) ? GS_BUILD_MIPMAPS : 0);
	obs_leave_graphics();

	if (!m_texture)
		throw std::runtime_error("Failed to create texture.");
}

GS::Texture::Texture(uint32_t size, gs_color_format format, uint32_t mip_levels, const uint8_t **mip_data, uint32_t flags) : m_destroy(true) {
	if (size == 0)
		throw std::logic_error("size must be at least 1");
	if (mip_levels == 0)
		throw std::logic_error("mip_levels must be at least 1");
	if (!mip_data)
		throw std::logic_error("mip_data is invalid");

	if (mip_levels > 1 || flags & Flags::BuildMipMaps) {
		bool isPOT = (pow(2, (int64_t)floor(log(size) / log(2))) == size);
		if (!isPOT)
			throw std::logic_error("mip mapping requires power of two dimensions");
	}

	obs_enter_graphics();
	m_texture = gs_cubetexture_create(size, format, mip_levels, mip_data, (flags & Flags::Dynamic) ? GS_DYNAMIC : 0 | (flags & Flags::BuildMipMaps) ? GS_BUILD_MIPMAPS : 0);
	obs_leave_graphics();

	if (!m_texture)
		throw std::runtime_error("Failed to create texture.");
}

GS::Texture::Texture(std::string file) : m_destroy(true) {
	struct stat st;
	if (os_stat(file.c_str(), &st) != 0)
		throw Plugin::file_not_found_error(file);

	obs_enter_graphics();
	m_texture = gs_texture_create_from_file(file.c_str());
	obs_leave_graphics();

	if (!m_texture)
		throw Plugin::io_error("Failed to load texture.", file);
}

GS::Texture::~Texture() {
	if (m_texture && m_destroy) {
		obs_enter_graphics();
		switch (gs_get_texture_type(m_texture)) {
			case GS_TEXTURE_2D:
				gs_texture_destroy(m_texture);
				break;
			case GS_TEXTURE_3D:
				gs_voltexture_destroy(m_texture);
				break;
			case GS_TEXTURE_CUBE:
				gs_cubetexture_destroy(m_texture);
				break;
		}
		m_texture = nullptr;
		obs_leave_graphics();
	}
}

void GS::Texture::Load(int unit) {
	obs_enter_graphics();
	gs_load_texture(m_texture, unit);
	obs_leave_graphics();
}

gs_texture_t* GS::Texture::GetObject() {
	return m_texture;
}
