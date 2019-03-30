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


const size_t GS::Texture::MAX_POOL_SIZE = 10;
std::map<std::string, std::pair<size_t, gs_texture_t*> > GS::Texture::pool;
gs_texture_t* GS::Texture::empty_texture = nullptr;


gs_texture_t *GS::Texture::get_empty_texture() {
	return empty_texture;
}

void GS::Texture::init_empty_texture() {
	obs_enter_graphics();
	if (empty_texture == nullptr)
	{
		const uint8_t *zero_tex[1];
		zero_tex[0] = new uint8_t[4]();

		empty_texture = gs_texture_create(1, 1, GS_RGBA, 1, (const uint8_t **)&zero_tex, 0);

		delete zero_tex[0];
	}
	obs_leave_graphics();
}

void GS::Texture::add_to_cache(std::string name, gs_texture_t *texture) {

	auto it = pool.find(name);
	if (it != pool.end() && it->second.second != nullptr) {
		throw "Incorrect use of add_to_cache, before calling load_from_cache";
	}

	if (pool.size() < MAX_POOL_SIZE)
	{
		//blog(LOG_DEBUG, "Caching texture: %s", name.c_str());
		pool[name] = std::make_pair(1, texture);
	}
	else
	{
		// remove the least re-used texture
		auto min_it = pool.begin();
		size_t min_ref = min_it->second.first;
		for (auto it = pool.begin(); it != pool.end(); it++)
		{
			if (min_ref > it->second.first)
			{
				min_ref = it->second.first;
				min_it = it;
			}
		}
		pool.erase(min_it);
		//blog(LOG_DEBUG, "Pool full. Removing least used texture: %s, #ref = %d", min_it->first.c_str(), min_it->second.first);

		pool[name] = std::make_pair(1, texture);
	}
}

void GS::Texture::load_from_cache(std::string name, gs_texture_t **texture_ptr) {
	if (pool.find(name) != pool.end()) {
		//blog(LOG_DEBUG, "Re-using texture: %s", name.c_str());
		pool[name].first++;
		*texture_ptr = pool[name].second;
	}
	else
		*texture_ptr = nullptr;
}

void GS::Texture::unload_texture(std::string name, gs_texture_t *texture) {
	// only destroy if the texture is not managed by the pool
	if (pool.find(name) == pool.end()) {
		//blog(LOG_DEBUG, "Destroying unmanaged texture");
		obs_enter_graphics();
		if (texture) {
			switch (gs_get_texture_type(texture)) {
			case GS_TEXTURE_2D:
				gs_texture_destroy(texture);
				break;
			case GS_TEXTURE_3D:
				gs_voltexture_destroy(texture);
				break;
			case GS_TEXTURE_CUBE:
				gs_cubetexture_destroy(texture);
				break;
			}
		}
		obs_leave_graphics();
		return;
	}

	//blog(LOG_DEBUG, "Delaying destroying managed texture: %s", name.c_str());

}

void GS::Texture::destroy_pool() {
	obs_enter_graphics();
	for (auto &ent : pool) {
		//blog(LOG_DEBUG, "POOL DESTROY: destroying managed texture: %s", ent.first.c_str());
		if (ent.second.second) {
			switch (gs_get_texture_type(ent.second.second)) {
			case GS_TEXTURE_2D:
				gs_texture_destroy(ent.second.second);
				break;
			case GS_TEXTURE_3D:
				gs_voltexture_destroy(ent.second.second);
				break;
			case GS_TEXTURE_CUBE:
				gs_cubetexture_destroy(ent.second.second);
				break;
			}
		}
		ent.second.second = nullptr;
	}
	obs_leave_graphics();
	// destroy empty texture as well
	obs_enter_graphics();
	if (empty_texture)
	{
		gs_texture_destroy(empty_texture);
		empty_texture = nullptr;
	}
	obs_leave_graphics();
}

GS::Texture::Texture(uint32_t width, uint32_t height, gs_color_format format, uint32_t mip_levels, const uint8_t **mip_data, uint32_t flags) : m_destroy(true) {
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

GS::Texture::Texture(uint32_t width, uint32_t height, uint32_t depth, gs_color_format format, uint32_t mip_levels, const uint8_t **mip_data, uint32_t flags) : m_destroy(true) {
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

GS::Texture::Texture(std::string name,uint32_t size, gs_color_format format, uint32_t mip_levels, const uint8_t **mip_data, uint32_t flags) : m_destroy(true) {
	m_name = name;
	obs_enter_graphics();
	GS::Texture::load_from_cache(m_name, &m_texture);
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

		GS::Texture::add_to_cache(m_name, m_texture);
	}
	obs_leave_graphics();
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
	if(m_destroy)
		GS::Texture::unload_texture(m_name, m_texture);
}

void GS::Texture::Load(int unit) {
	obs_enter_graphics();
	gs_load_texture(m_texture, unit);
	obs_leave_graphics();
}

gs_texture_t* GS::Texture::GetObject() {
	return m_texture;
}
