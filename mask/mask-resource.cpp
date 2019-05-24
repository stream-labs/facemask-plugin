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

#include "mask-resource.h"
#include "plugin/exceptions.h"
#include "plugin/plugin.h"
#include "mask.h"
#include "mask-resource-effect.h"
#include "mask-resource-emitter.h"
#include "mask-resource-image.h"
#include "mask-resource-material.h"
#include "mask-resource-mesh.h"
#include "mask-resource-model.h"
#include "mask-resource-morph.h"
#include "mask-resource-skinned-model.h"
#include "mask-resource-sequence.h"
#include "mask-resource-sound.h"
#include "mask-resource-light.h"
#include "mask-resource-animation.h"
#include "mask-resource-animation-target.h"

#include <Windows.h>


Mask::Resource::IBase::IBase(Mask::MaskData* parent, std::string name) {
	m_parent = parent;
	m_name = name;
	std::hash<std::string> hasher;
	m_id = hasher(m_name);
}

Mask::Resource::IBase::~IBase() {

}

std::shared_ptr<Mask::Resource::IBase> Mask::Resource::IBase::Load(Mask::MaskData* parent, std::string name, obs_data_t* data, Cache *cache) {
	static const char* const S_TYPE = "type";

	if (!obs_data_has_user_value(data, S_TYPE)) {
		PLOG_ERROR("Resource '%s' is missing type.", name.c_str());
		throw std::logic_error("Resource is missing type.");
	}

	// yield
	::Sleep(0);

	std::string type = obs_data_get_string(data, S_TYPE);
	if (type == "image") {
		// Image
		return std::make_shared<Mask::Resource::Image>(parent, name, data, cache);
	} 
	else if (type == "sequence") {
		// Image Sequence
		return std::make_shared<Mask::Resource::Sequence>(parent, name, data);
	}
	else if (type == "effect") {
		// Effect Shader
		return std::make_shared<Mask::Resource::Effect>(parent, name, data, cache);
	} 
	else if (type == "material") {
		// Material (Combines Images, Effects)
		return std::make_shared<Mask::Resource::Material>(parent, name, data);
	} 
	else if (type == "mesh") {
		// Mesh
		return std::make_shared<Mask::Resource::Mesh>(parent, name, data);
	}
	else if (type == "model") {
		// Model
		return std::make_shared<Mask::Resource::Model>(parent, name, data);
	}
	else if (type == "morph") {
		// Morph
		return std::make_shared<Mask::Resource::Morph>(parent, name, data);
	}
	else if (type == "skinned-model") {
		// Skinned Model
		return std::make_shared<Mask::Resource::SkinnedModel>(parent, name, data);
	} 
	else if (type == "emitter") {
		// Particle Emitter
		return std::make_shared<Mask::Resource::Emitter>(parent, name, data);
	} 
	else if (type == "light") {
		// Light
		return std::make_shared<Mask::Resource::Light>(parent, name, data);
	}
	else if (type == "animation") {
		// Animation
		return std::make_shared<Mask::Resource::Animation>(parent, name, data);
	}
	else if (type == "animation-target-list") {
		// Animation
		return std::make_shared<Mask::Resource::AnimationTarget>(parent, name, data);
	}
	else if (type == "sound") {
		// Sound (not supported)
		//return std::make_shared<Mask::Resource::Effect>(parent, name, data);
	}
	return nullptr;
}

std::shared_ptr<Mask::Resource::IBase> Mask::Resource::IBase::LoadDefault(Mask::MaskData* parent, std::string name, Cache *cache) {
	std::shared_ptr<Mask::Resource::IBase> p(nullptr);

	static const std::map<std::string, std::string> g_defaultImages = {
		{ "imageNull", "resources/null.png" },
		{ "imageWhite", "resources/white.png" },
		{ "imageBlack", "resources/black.png" },
		{ "imageRed", "resources/red.png" },
		{ "imageGreen", "resources/green.png" },
		{ "imageBlue", "resources/blue.png" },
		{ "imageYellow", "resources/yellow.png" },
		{ "imageMagenta", "resources/magenta.png" },
		{ "imageCyan", "resources/cyan.png" },
	};

	static const std::map<std::string, std::string> g_defaultMeshes = {
		{ "meshTriangle", "resources/triangle.notobj" },
		{ "meshQuad", "resources/quad.notobj" },
		{ "meshCube", "resources/cube.notobj" },
		{ "meshSphere", "resources/sphere.notobj" },
		{ "meshCylinder", "resources/cylinder.notobj" },
		{ "meshPyramid", "resources/pyramid.notobj" },
		{ "meshTorus", "resources/torus.notobj" },
		{ "meshCone", "resources/cone.notobj" },
		{ "meshHead", "resources/head.notobj" },
	};

	static const std::map<std::string, std::string> g_defaultEffects = {
		{ "effectDefault", "effects/default.effect" },
		{ "effectPhong", "effects/phong.effect" },
		{ "PBR", "effects/pbr.effect" },
	};

	obs_data_t *g_templates = nullptr;
	cache->load_permanent("templates", (void **)&g_templates);
	// load once
	if (g_templates == nullptr) {
		// load templates file
		char* f = obs_module_file("resources/templates.json");
		g_templates = obs_data_create_from_json_file(f);
		cache->add_permanent(CacheableType::OBSData, "templates", g_templates);
		bfree(f);
	}

	// yield
	::Sleep(0);

	// image?
	if (g_defaultImages.find(name) != g_defaultImages.end()) {
		char* f = obs_module_file(g_defaultImages.at(name).c_str());
		p = std::make_shared<Mask::Resource::Image>
			(parent, name, std::string(f), cache);
		bfree(f);
	}
	// mesh?
	if (g_defaultMeshes.find(name) != g_defaultMeshes.end()) {
		char* f = obs_module_file(g_defaultMeshes.at(name).c_str());
		p = std::make_shared<Mask::Resource::Mesh>
			(parent, name, std::string(f));
		bfree(f);
	}
	// effect?
	if (g_defaultEffects.find(name) != g_defaultEffects.end()) {
		char* f = obs_module_file(g_defaultEffects.at(name).c_str());
		p = std::make_shared<Mask::Resource::Effect>
			(parent, name, std::string(f), cache);
		bfree(f); 
	}
	// template?
	if (obs_data_has_user_value(g_templates, name.c_str())) {
		obs_data_t* template_data = obs_data_get_obj(g_templates, name.c_str());
		p = Load(parent, name, template_data, cache);
		obs_data_release(template_data);
	}

	return p;
}


// ------------------------------------------------------------------------- //
// Cache Pools
// ------------------------------------------------------------------------- //

const size_t Mask::Resource::Cache::POOL_SIZE = 10;

bool Mask::Resource::Cache::add_permanent(CacheableType resource_type, std::string name, void *resource) {
	auto pool_it = permanent_cache.find(name);
	if (pool_it == permanent_cache.end()) {
		permanent_cache[name] = std::make_pair(resource_type, resource);
		return true;
	}
	return false;
}

bool Mask::Resource::Cache::add(CacheableType resource_type, std::string name, void *resource) {
	auto pool_it = pool_map.find(resource_type);
	if (pool_it == pool_map.end()) {
		pool_map[resource_type] = CachePool();
		pool_it = pool_map.find(resource_type);
	}
	CachePool &pool = pool_it->second;

	auto it = pool.find(name);
	if (it != pool.end()) {
		if (it->second.resource != nullptr)
		{
			if (it->second.active_count == 0) {
				// no one is using the old one
				// probably a name clash from an older mask load
				// can safely replace
				//blog(LOG_DEBUG, "Caching resource (replacing inactive older version): %s", name.c_str());
				pool[name] = CacheItem(resource);
				return true;
			}
			else
				throw "Incorrect use of add before calling load";
		}
		else {
			//blog(LOG_DEBUG, "Caching resource (re-bind): %s", name.c_str());
			pool[name] = CacheItem(resource);
			return true;
		}
	}
	else if (pool.size() < POOL_SIZE)
	{
		//blog(LOG_DEBUG, "Caching resource: %s", name.c_str());
		pool[name] = CacheItem(resource);
		return true;
	}
	else
	{
		// remove the least re-used resource
		auto min_it = pool.begin();
		size_t min_ref = min_it->second.use_count;
		for (auto it = pool.begin(); it != pool.end(); it++)
		{
			if (min_ref > it->second.use_count)
			{
				min_ref = it->second.use_count;
				min_it = it;
			}
		}
		// if the least re-used resource is actively used
		// by 2 or more, we cannot remove it
		// let's search for the currently least active resource
		if (min_it->second.active_count > 2)
		{
			min_it = pool.begin();
			min_ref = min_it->second.active_count;
			for (auto it = pool.begin(); it != pool.end(); it++)
			{
				if (min_ref > it->second.active_count)
				{
					min_ref = it->second.active_count;
					min_it = it;
				}
			}
		}

		//blog(LOG_DEBUG, "Pool full. Removing least used/active resource: %s, #active = %d, #use = %d",
		//	min_it->first.c_str(), min_it->second.active_count, min_it->second.use_count);

		if (min_it->second.active_count == 1)
		{
			// only remove from the pool
			// the one active user will
			// correctly destruct the resource
			pool.erase(min_it);
		}
		else if (min_it->second.active_count == 0)
		{
			// no one were using this resource
			// let's destruct it ourselves
			destruct_by_type(min_it->second.resource, resource_type);
			pool.erase(min_it);
		}
		else
		{
			// sorry we cannot cache
			// the pool is full and
			// no one can leave it currently
			//blog(LOG_DEBUG, "Caching failed because the pool is full, and no one is willing to leave. Resource: %s", name.c_str());
			return false;
		}

		//blog(LOG_DEBUG, "Caching resource: %s", name.c_str());

		pool[name] = CacheItem(resource);

		return true;
	}
}

void Mask::Resource::Cache::load(CacheableType resource_type, std::string name, void **resource_ptr) {
	auto pool_it = pool_map.find(resource_type);
	if (pool_it == pool_map.end()) {
		*resource_ptr = nullptr;
		return;
	}
	CachePool &pool = pool_it->second;
	if (pool.find(name) != pool.end()) {
		//blog(LOG_DEBUG, "Re-using resource: %s", name.c_str());
		pool[name].active_count++;
		pool[name].use_count++;
		*resource_ptr = pool[name].resource;
	}
	else
		*resource_ptr = nullptr;
}

void Mask::Resource::Cache::load_permanent(std::string name, void **resource_ptr) {
	auto item = permanent_cache.find(name);
	if (item != permanent_cache.end()) {
		*resource_ptr = item->second.second;
	}
	else
		*resource_ptr = nullptr;
}

void Mask::Resource::Cache::destruct_by_type(void *resource, CacheableType resource_type) {
	if (resource == nullptr) return;

	obs_enter_graphics();
	switch (resource_type)
	{
	case CacheableType::Texture:
	{
		gs_texture_t *texture = reinterpret_cast<gs_texture_t*>(resource);

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
	break;
	case CacheableType::Effect:
	{
		gs_effect_t *effect = reinterpret_cast<gs_effect_t*>(resource);
		gs_effect_destroy(effect);
	}
	break;
	case CacheableType::OBSData:
	{
		obs_data_t *data = reinterpret_cast<obs_data_t*>(resource);
		obs_data_release(data);
	}
	break;
	}
	obs_leave_graphics();
}

void Mask::Resource::Cache::try_destroy_resource(std::string name, void *resource, CacheableType resource_type) {
	// only destroy if the resource is not managed by the pool
	auto pool_it = pool_map.find(resource_type);
	if (pool_it == pool_map.end()) return;
	CachePool &pool = pool_it->second;

	auto it = pool.find(name);
	if (it == pool.end()) {
		destruct_by_type(resource, resource_type);
	}
	else {
		//blog(LOG_DEBUG, "Delaying destroying managed resource: %s, #active = %d", name.c_str(), it->second.active_count);
		it->second.active_count--;
	}
}

void Mask::Resource::Cache::destroy() {
	obs_enter_graphics();

	// destroy pool-managed caches
	for (auto &pool_ent : pool_map)
	{
		CachePool &pool = pool_ent.second;
		for (auto &ent : pool) {
			//blog(LOG_DEBUG, "Cache Destroy: destroying managed resource: %s", ent.first.c_str());
			destruct_by_type(ent.second.resource, pool_ent.first);
			ent.second.resource = nullptr;
		}
	}

	// destroy permanent cache
	for (auto &ent : permanent_cache) {
		//blog(LOG_DEBUG, "Cache Destroy: destroying permanent resource: %s", ent.first.c_str());
		// ent.second = pair<CacheableType, void *resource>
		destruct_by_type(ent.second.second, ent.second.first);
		ent.second.second = nullptr;
	}

	obs_leave_graphics();

}

