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

#include "mask.h"
#include "mask-resource-model.h"
#include "mask-resource-material.h"
#include "mask-resource-animation.h"
#include "exceptions.h"
#include "strings.h"
#include "plugin.h"
#include <queue>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/graphics/graphics.h>
	#include <libobs/graphics/axisang.h>
	#pragma warning( pop )
}

static const char* const JSON_METADATA_NAME = "name";
static const char* const JSON_METADATA_DESCRIPTION = "description";
static const char* const JSON_METADATA_AUTHOR = "author";
static const char* const JSON_METADATA_WEBSITE = "website";
static const char* const JSON_RESOURCES = "resources";
static const char* const JSON_PARTS = "parts";
static const char* const JSON_TYPE = "type";
static const char* const JSON_ANIMATION = "animation";

static const int NUM_DRAW_BUCKETS = 1024;
static const float BUCKETS_MAX_Z = 10.0f;
static const float BUCKETS_MIN_Z = -100.0f;

Mask::MaskData::MaskData() : m_data(nullptr) {
	m_drawBuckets = new Mask::SortedDrawObject*[NUM_DRAW_BUCKETS];
	ClearSortedDrawObjects();
}

Mask::MaskData::~MaskData() {
	delete[] m_drawBuckets;
	Clear();
}

void Mask::MaskData::Clear() {
	m_parts.clear();
	m_resources.clear();
	if (m_data) {
		obs_data_release(m_data);
		m_data = nullptr;
	}
}

void Mask::MaskData::Load(const std::string& file) {
	if (m_data) {
		obs_data_release(m_data);
		m_data = nullptr;
	}
	m_data = obs_data_create_from_json_file(file.c_str());
	if (!m_data)
		throw std::ios_base::failure(file);

	// World Part
	m_partWorld = std::make_shared<Mask::Part>();
	AddPart("world", m_partWorld);
	// 0,-50,0 is roughly where your face is...consider this origin of lights
	vec3_set(&m_partWorld->position, 0, 0, -50);
	m_partWorld->localdirty = true;

	// Metadata
	if (obs_data_has_user_value(m_data, JSON_METADATA_NAME))
		m_metaData.name = obs_data_get_string(m_data, JSON_METADATA_NAME);
	else
		m_metaData.name = "unknown";

	if (obs_data_has_user_value(m_data, JSON_METADATA_DESCRIPTION))
		m_metaData.description = obs_data_get_string(m_data, JSON_METADATA_DESCRIPTION);
	else
		m_metaData.description = "";

	if (obs_data_has_user_value(m_data, JSON_METADATA_AUTHOR))
		m_metaData.author = obs_data_get_string(m_data, JSON_METADATA_AUTHOR);
	else
		m_metaData.author = "unknown";

	if (obs_data_has_user_value(m_data, JSON_METADATA_WEBSITE))
		m_metaData.website = obs_data_get_string(m_data, JSON_METADATA_WEBSITE);
	else
		m_metaData.website = "";

	// Loading is implemented through lazy loading. Only things that are used by
	// parts and referenced resources will be available, which should improve
	// load speed and also reduce memory footprint.
	obs_data_item_t* partDataItem = obs_data_item_byname(m_data, JSON_PARTS);
	if (!partDataItem) {
		PLOG_ERROR("Parts section is missing in mask definition '%s'.",
			file.c_str());
		return;
	}
	if (obs_data_item_gettype(partDataItem) != obs_data_type::OBS_DATA_OBJECT) {
		PLOG_ERROR("Parts section is malformed in mask definition '%s'.",
			file.c_str());
		return;
	}
	obs_data_t* partData = obs_data_item_get_obj(partDataItem);
	if (!partData) {
		PLOG_ERROR("Internal error while reading parts section in mask definition '%s'.",
			file.c_str());
		return;
	}
	for (obs_data_item_t* itm = obs_data_first(partData); itm; obs_data_item_next(&itm)) {
		std::string name = obs_data_item_get_name(itm);
		GetPart(name);
	}

	// Except for animations. 
	obs_data_t* resources = obs_data_get_obj(m_data, JSON_RESOURCES);
	for (obs_data_item_t* el = obs_data_first(resources); el; obs_data_item_next(&el)) {
		std::string resourceName = obs_data_item_get_name(el);
		obs_data_t* resd = obs_data_item_get_obj(el);
		if (!resd)
			continue;
		if (!obs_data_has_user_value(resd, JSON_TYPE))
			continue;
		std::string resourceType = obs_data_get_string(resd, JSON_TYPE);
		if (resourceType == "animation") {
			m_animations.emplace(resourceName, 
				std::dynamic_pointer_cast<Resource::Animation>(GetResource(resourceName)));
		}
	}
}

void Mask::MaskData::AddResource(const std::string& name, std::shared_ptr<Mask::Resource::IBase> resource) {
	if (name.length() == 0)
		throw std::invalid_argument("name must be at least one character long");
	if (resource == nullptr)
		throw std::invalid_argument("resource is a null pointer");
	if (m_resources.count(name) > 0)
		throw std::invalid_argument("resource is already present");

	m_resources.emplace(name, resource);
}

std::shared_ptr<Mask::Resource::IBase> Mask::MaskData::GetResource(const std::string& name) {
	if (name.length() == 0)
		throw std::invalid_argument("name must be at least one character long");

	auto kv = m_resources.find(name);
	if (kv != m_resources.end())
		return kv->second;

	// Default Resources
	std::shared_ptr<Mask::Resource::IBase> p = Resource::IBase::LoadDefault(this, name);
	if (p) {
		this->AddResource(name, p);
		return p;
	}

	// Lazy Loaded
	obs_data_item_t* resources = obs_data_item_byname(m_data, JSON_RESOURCES);
	if (!resources) return nullptr;
	if (obs_data_item_gettype(resources) != OBS_DATA_OBJECT) return nullptr;
	obs_data_t* resd = obs_data_item_get_obj(resources);
	if (!resd) return nullptr;
	obs_data_item_t* element = obs_data_item_byname(resd, name.c_str());
	if (!element) return nullptr;
	if (obs_data_item_gettype(element) != OBS_DATA_OBJECT) return nullptr;
	obs_data_t* elmd = obs_data_item_get_obj(element);
	if (!elmd) return nullptr;
	try {
		auto res = Resource::IBase::Load(this, name, elmd);
		if (res) {
			PLOG_DEBUG("Loading resource %s...", name.c_str());
			this->AddResource(name, res);
			return res;
		}
	} catch (...) {
		PLOG_DEBUG("Resource %s has THROWN AN EXCEPTION. MASK DID NOT LOAD CORRECTLY.", name.c_str());
		return nullptr;
	}
	return nullptr;
}

std::shared_ptr<Mask::Resource::IBase> Mask::MaskData::RemoveResource(const std::string& name) {
	if (name.length() == 0)
		throw std::invalid_argument("name must be at least one character long");

	std::shared_ptr<Mask::Resource::IBase> el = nullptr;
	auto kv = m_resources.find(name);
	if (kv != m_resources.end()) {
		el = kv->second;
		m_resources.erase(kv);
	}
	return el;
}

void Mask::MaskData::AddPart(const std::string& name, std::shared_ptr<Mask::Part> part) {
	if (name.length() == 0)
		throw std::invalid_argument("name must be at least one character long");
	if (part == nullptr)
		throw std::invalid_argument("part is a null pointer");
	if (m_parts.count(name) > 0)
		throw std::invalid_argument("part is already present");

	part->mask = this;
	std::hash<std::string> hasher;
	part->hash_id = hasher(name);
	m_parts.emplace(name, part);
}

std::shared_ptr<Mask::Part> Mask::MaskData::GetPart(const std::string& name) {
	if (name.length() == 0)
		throw std::invalid_argument("name must be at least one character long");

	auto kv = m_parts.find(name);
	if (kv != m_parts.end()) {
		return kv->second;
	}

	// Lazy Loaded
	obs_data_item_t* parts = obs_data_item_byname(m_data, JSON_PARTS);
	if (!parts) return nullptr;
	if (obs_data_item_gettype(parts) != OBS_DATA_OBJECT) return nullptr;
	obs_data_t* resd = obs_data_item_get_obj(parts);
	if (!resd) return nullptr;
	obs_data_item_t* element = obs_data_item_byname(resd, name.c_str());
	if (!element) return nullptr;
	if (obs_data_item_gettype(element) != OBS_DATA_OBJECT) return nullptr;
	obs_data_t* elmd = obs_data_item_get_obj(element);
	if (!elmd) return nullptr;
	return LoadPart(name, elmd);
}

std::shared_ptr<Mask::Part> Mask::MaskData::RemovePart(const std::string& name) {
	if (name.length() == 0)
		throw std::invalid_argument("name must be at least one character long");

	std::shared_ptr<Mask::Part> el = nullptr;
	auto kv = m_parts.find(name);
	if (kv != m_parts.end()) {
		el = kv->second;
		m_parts.erase(kv);
	}
	return el;
}

void  Mask::MaskData::ClearSortedDrawObjects() {
	SortedDrawObject** sdo = m_drawBuckets;
	for (unsigned int i = 0; i < NUM_DRAW_BUCKETS; i++) {
		*sdo = nullptr;
		sdo++;
	}
}

void  Mask::MaskData::AddSortedDrawObject(SortedDrawObject* obj) {
	float z = obj->SortDepth();
	if (z > BUCKETS_MAX_Z)
		z = BUCKETS_MAX_Z;
	if (z < BUCKETS_MIN_Z)
		z = BUCKETS_MIN_Z;
	z = (z - BUCKETS_MIN_Z) / (BUCKETS_MAX_Z - BUCKETS_MIN_Z);
	int idx = (int)(z * (float)(NUM_DRAW_BUCKETS - 1));
	obj->nextDrawObject = m_drawBuckets[idx];
	obj->instanceId = instanceDatas.CurrentId();
	m_drawBuckets[idx] = obj;
}

void  Mask::MaskData::PartCalcMatrix(std::shared_ptr<Mask::Part> _part) {
	register Mask::Part* part = _part.get();

	// Only if we're dirty
	if (part->dirty) {
		if (part->localdirty) {
		
			// Calculate local matrix 
			matrix4_identity(&part->local);
			matrix4_scale3f(&part->local, &part->local,
				part->scale.x, part->scale.y, part->scale.z);
			if (part->isquat) {
				matrix4 qm;
				matrix4_from_quat(&qm, &part->qrotation);
				matrix4_mul(&part->local, &part->local, &qm);
			}
			else {
				matrix4_rotate_aa4f(&part->local, &part->local,
					1.0f, 0.0f, 0.0f, part->rotation.x);
				matrix4_rotate_aa4f(&part->local, &part->local,
					0.0f, 1.0f, 0.0f, part->rotation.y);
				matrix4_rotate_aa4f(&part->local, &part->local,
					0.0f, 0.0f, 1.0f, part->rotation.z);
			}
			matrix4_translate3f(&part->local, &part->local,
				part->position.x, part->position.y, part->position.z);
			part->localdirty = false;
		}

		// Calculate global
		matrix4_copy(&part->global, &part->local);
		if (part->parent) {
			PartCalcMatrix(part->parent);
			matrix4_mul(&part->global, &part->global,
				&part->parent->global);
		} 
		
		part->dirty = false;
	}
}

void Mask::MaskData::Tick(float time) {
	// update animations with the first Part
	for (auto aakv : m_animations) {
		if (aakv.second) {
			Part* p = nullptr;
			for (auto kv : m_parts) {
				p = kv.second.get();
				break;
			}
			aakv.second->Update(p, time);
		}
	}

	// mark parts dirty
	for (auto kv : m_parts) {
		kv.second->dirty = true;
	}
	// calculate transforms 
	for (auto kv : m_parts) {
		PartCalcMatrix(kv.second);
	}
	// update part resources
	for (auto kv : m_parts) {
		instanceDatas.Push(kv.second->hash_id);
		for (auto it = kv.second->resources.begin();
			it != kv.second->resources.end(); it++) {
			(*it)->Update(kv.second.get(), time);
		}
		instanceDatas.Pop();
	}
}

void Mask::MaskData::Render(bool depthOnly) {

	ClearSortedDrawObjects();

	// OPAQUE
	for (auto kv : m_parts) {
		if (kv.second->resources.size() == 0)
			continue;
		instanceDatas.Push(kv.second->hash_id);
		gs_matrix_push();
		gs_matrix_mul(&kv.second->global);
		for (auto it = kv.second->resources.begin();
			it != kv.second->resources.end(); it++) {
			if ((*it)->IsDepthOnly() == depthOnly) {
				(*it)->Render(kv.second.get());
			}
		}
		gs_matrix_pop();
		instanceDatas.Pop();
	}

	// TRANSPARENT
	if (!depthOnly) {
		for (unsigned int i = 0; i < NUM_DRAW_BUCKETS; i++) {
			SortedDrawObject* sdo = m_drawBuckets[i];
			while (sdo) {
				Part* part = sdo->sortDrawPart;
				instanceDatas.PushDirect(sdo->instanceId);
				gs_matrix_push();
				gs_matrix_mul(&part->global);
				sdo->SortedRender();
				gs_matrix_pop();
				instanceDatas.Pop();
				sdo = sdo->nextDrawObject;
			}
		}
	}
}

static const char* const S_POSITION = "position";
static const char* const S_ROTATION = "rotation";
static const char* const S_QROTATION = "qrotation";
static const char* const S_SCALE = "scale";
static const char* const S_RESOURCE = "resource";
static const char* const S_RESOURCES = "resources";
static const char* const S_PARENT = "parent";


std::shared_ptr<Mask::Part> Mask::MaskData::LoadPart(std::string name, obs_data_t* data) {
	std::shared_ptr<Part> current = std::make_shared<Part>();
	if (obs_data_has_user_value(data, S_POSITION))
		obs_data_get_vec3(data, S_POSITION, &current->position);

	if (obs_data_has_user_value(data, S_ROTATION))
		obs_data_get_vec3(data, S_ROTATION, &current->rotation);

	if (obs_data_has_user_value(data, S_QROTATION)) {
		current->isquat = true;
		obs_data_get_quat(data, S_QROTATION, &current->qrotation);
	}
	
	if (obs_data_has_user_value(data, S_SCALE))
		obs_data_get_vec3(data, S_SCALE, &current->scale);
	
	// Resources
	if (obs_data_has_user_value(data, S_RESOURCE)) {
		std::string resourceName = obs_data_get_string(data, S_RESOURCE);
		current->resources.push_back(GetResource(resourceName));
	}
	if (obs_data_has_user_value(data, S_RESOURCES)) {
		obs_data_item_t* rezItem = obs_data_item_byname(data, S_RESOURCES);
		if (obs_data_item_gettype(rezItem) != obs_data_type::OBS_DATA_OBJECT) {
			PLOG_ERROR("Bad resources section in '%s'.", name.c_str());
			return current;
		}
		obs_data_t* rezData = obs_data_item_get_obj(rezItem);
		if (!rezData) {
			PLOG_ERROR("Bad resources section in '%s'.", name.c_str());
			return current;
		}
		for (obs_data_item_t* itm = obs_data_first(rezData); itm; obs_data_item_next(&itm)) {
			std::string resourceName = obs_data_item_get_string(itm);
			current->resources.push_back(GetResource(resourceName));
		}
	}

	// Parent 
	if (obs_data_has_user_value(data, S_PARENT)) {
		std::string parentName = obs_data_get_string(data, S_PARENT);
		// root node is deprecated, but might still be kicking around
		if (parentName != "root") {
			current->parent = GetPart(parentName);
		}
	}

	this->AddPart(name, current);
	return current;
}

