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

GS::Texture::Texture(uint32_t width, uint32_t height, gs_color_format format, uint32_t mip_levels, const uint8_t **mip_data, uint32_t flags,
	Cache *cache) : m_destroy(true), m_cache(cache) {
	m_name = ""; // will not participate in caching
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

GS::Texture::Texture(uint32_t width, uint32_t height, uint32_t depth, gs_color_format format, uint32_t mip_levels, const uint8_t **mip_data, uint32_t flags,
	Cache *cache) : m_destroy(true), m_cache(cache) {
	m_name = ""; // will not participate in caching
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

GS::Texture::Texture(std::string name,uint32_t size, gs_color_format format, uint32_t mip_levels, const uint8_t **mip_data, uint32_t flags,
	Cache *cache) : m_destroy(true), m_cache(cache) {
	m_name = name;
	obs_enter_graphics();
	m_texture = nullptr;
	if (m_cache != nullptr)
		m_cache->load(CacheableType::Texture, m_name, (void **)&m_texture);
	if (m_texture == nullptr)
	{
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

		m_texture = gs_cubetexture_create(size, format, mip_levels, mip_data, (flags & Flags::Dynamic) ? GS_DYNAMIC : 0 | (flags & Flags::BuildMipMaps) ? GS_BUILD_MIPMAPS : 0);

		if (!m_texture)
			throw std::runtime_error("Failed to create texture.");

		if (!m_cache->add(CacheableType::Texture, m_name, (void *)m_texture))
			blog(LOG_WARNING, "Caching texture failed: %s", m_name.c_str());

	}
	obs_leave_graphics();
}

GS::Texture::Texture(std::string file, Cache *cache) : m_destroy(true), m_cache(cache) {
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
	if(m_destroy)
		m_cache->try_destroy_resource(m_name, m_texture,
			CacheableType::Texture);
}

void GS::Texture::Load(int unit) {
	obs_enter_graphics();
	gs_load_texture(m_texture, unit);
	obs_leave_graphics();
}

gs_texture_t* GS::Texture::GetObject() {
	return m_texture;
}
