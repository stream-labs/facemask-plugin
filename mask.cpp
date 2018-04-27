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
#include "mask-resource-sequence.h"
#include "exceptions.h"
#include "strings.h"
#include "plugin.h"
#include "utils.h"
#include <queue>
#include <thread>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/graphics/graphics.h>
	#include <libobs/graphics/axisang.h>
	#pragma warning( pop )
}


Mask::Part::Part(std::shared_ptr<Part> p_parent,
	std::shared_ptr<Resource::IBase> p_resource) :
	parent(p_parent), 
	localdirty(true), dirty(true), isquat(false) {
	vec3_zero(&position);
	vec3_zero(&rotation);
	vec3_set(&scale, 1, 1, 1);
	quat_identity(&qrotation);
	if (p_resource)
		resources.push_back(p_resource);
	matrix4_identity(&local);
	matrix4_identity(&global);
}


void Mask::Part::SetAnimatableValue(float v, 
	Mask::Resource::AnimationChannelType act) {
	// set value
	switch (act) {
	case Mask::Resource::PART_POSITION_X:
		position.x = v;
		break;
	case Mask::Resource::PART_POSITION_Y:
		position.y = v;
		break;
	case Mask::Resource::PART_POSITION_Z:
		position.z = v;
		break;
	case Mask::Resource::PART_QROTATION_X:
		qrotation.x = v;
		break;
	case Mask::Resource::PART_QROTATION_Y:
		qrotation.y = v;
		break;
	case Mask::Resource::PART_QROTATION_Z:
		qrotation.z = v;
		break;
	case Mask::Resource::PART_QROTATION_W:
		qrotation.w = v;
		break;
	case Mask::Resource::PART_SCALE_X:
		scale.x = v;
		break;
	case Mask::Resource::PART_SCALE_Y:
		scale.y = v;
		break;
	case Mask::Resource::PART_SCALE_Z:
		scale.z = v;
		break;
	}
	localdirty = true;
}

static const char* const JSON_METADATA_NAME = "name";
static const char* const JSON_METADATA_DESCRIPTION = "description";
static const char* const JSON_METADATA_AUTHOR = "author";
static const char* const JSON_METADATA_WEBSITE = "website";
static const char* const JSON_RESOURCES = "resources";
static const char* const JSON_PARTS = "parts";
static const char* const JSON_TYPE = "type";
static const char* const JSON_ANIMATION = "animation";
static const char* const JSON_IS_INTRO = "is_intro";
static const char* const JSON_INTRO_FADE_TIME = "intro_fade_time";
static const char* const JSON_INTRO_DURATION = "intro_duration";


static const int NUM_DRAW_BUCKETS = 1024;
static const float BUCKETS_MAX_Z = 10.0f;
static const float BUCKETS_MIN_Z = -100.0f;

Mask::MaskData::MaskData() : m_data(nullptr), m_morph(nullptr), m_elapsedTime(0.0f) {
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
	m_morph = nullptr;
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

	// Intro Animation Fade Values
	if (obs_data_has_user_value(m_data, JSON_IS_INTRO))
		m_isIntroAnim = obs_data_get_bool(m_data, JSON_IS_INTRO);
	else
		m_isIntroAnim = false;

	if (obs_data_has_user_value(m_data, JSON_INTRO_FADE_TIME))
		m_introFadeTime = (float)obs_data_get_double(m_data, JSON_INTRO_FADE_TIME);
	else
		m_introFadeTime = 10.0f;

	if (obs_data_has_user_value(m_data, JSON_INTRO_DURATION))
		m_introDuration = (float)obs_data_get_double(m_data, JSON_INTRO_DURATION);
	else
		m_introDuration = 10.0f;

	std::this_thread::sleep_for(std::chrono::microseconds(1000));

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

	std::this_thread::sleep_for(std::chrono::microseconds(1000));

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

	std::this_thread::sleep_for(std::chrono::microseconds(1000));

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
			this->AddResource(name, res);
			return res;
		}
	} catch (...) {
		PLOG_DEBUG("Resource %s has THROWN AN EXCEPTION. MASK DID NOT LOAD CORRECTLY.", name.c_str());
		return nullptr;
	}
	return nullptr;
}

std::shared_ptr<Mask::Resource::IBase> Mask::MaskData::GetResource(Mask::Resource::Type type) {
	for (auto kv = m_resources.begin(); kv != m_resources.end(); kv++) {
		if (kv->second->GetType() == type) {
			return kv->second;
		}
	}
	return nullptr;
}

size_t Mask::MaskData::GetNumResources(Mask::Resource::Type type) {
	size_t count = 0;
	for (auto kv = m_resources.begin(); kv != m_resources.end(); kv++) {
		if (kv->second->GetType() == type) {
			count++;
		}
	}
	return count;
}

std::shared_ptr<Mask::Resource::IBase> Mask::MaskData::GetResource(Mask::Resource::Type type, int which) {
	int count = 0;
	for (auto kv = m_resources.begin(); kv != m_resources.end(); kv++) {
		if (kv->second->GetType() == type) {
			if (count == which) {
				return kv->second;
			}
			else {
				count++;
			}
		}
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
	
	// intro animation?
	if (m_isIntroAnim) {
		std::shared_ptr<Mask::AlphaInstanceData> aid =
			instanceDatas.GetData<Mask::AlphaInstanceData>(Mask::AlphaInstanceDataId);

		float DF = m_introDuration - m_introFadeTime;
		float t = 1.0f;

		// fading in?
		if (m_elapsedTime < m_introFadeTime) {
			t = m_elapsedTime / m_introFadeTime;
		}
		// drawing?
		else if (m_elapsedTime < DF) { 
			t = 1.0f;
		}
		// fading out?
		else if (m_elapsedTime < m_introDuration) {
			t = 1.0f - ((m_elapsedTime - DF) / m_introFadeTime);
		}
		else {
			t = 0.0f;
		}
		aid->alpha = Utils::hermite(t, 0.0f, 1.0f);
	}

	m_elapsedTime += time;
}

void Mask::MaskData::Render(bool depthOnly) {

	ClearSortedDrawObjects();

	gs_blend_state_push();
	gs_reset_blend_state();
	gs_enable_blending(true);
	gs_blend_function_separate(gs_blend_type::GS_BLEND_ONE,
		gs_blend_type::GS_BLEND_ZERO,
		gs_blend_type::GS_BLEND_ONE,
		gs_blend_type::GS_BLEND_ZERO);
	gs_enable_color(true, true, true, true);

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
		gs_blend_function_separate(gs_blend_type::GS_BLEND_SRCALPHA,
			gs_blend_type::GS_BLEND_INVSRCALPHA,
			gs_blend_type::GS_BLEND_ONE,
			gs_blend_type::GS_BLEND_ZERO);

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

	gs_blend_state_pop();
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

Mask::Resource::Morph* Mask::MaskData::GetMorph() {
	// cache the morph pointer so we dont constantly search for it
	if (m_morph == nullptr) {
		m_morph = std::dynamic_pointer_cast<Mask::Resource::Morph>
			(GetResource(Mask::Resource::Type::Morph)).get();
	}
	return m_morph;
}

bool Mask::MaskData::RenderMorphVideo(gs_texture* vidtex, uint32_t width, uint32_t height,
	const smll::TriangulationResult& trires) {

	bool didMorph = false;

	GetMorph();

	// Add an empty morph resource if they want to use the auto-green screen feature
	if (trires.autoBGRemoval && m_morph == nullptr) {
		std::string n("auto_morph");
		std::shared_ptr<Mask::Resource::IBase> r = 
			std::make_shared<Mask::Resource::Morph>(this, n);
		AddResource(n, r);
	}

	if (m_morph && trires.vertexBuffer) {
		didMorph = true;
		m_morph->RenderMorphVideo(vidtex, trires);
	}
	else {
		// Effects
		gs_effect_t* defaultEffect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

		// Draw the source video
		gs_enable_depth_test(false);
		gs_set_cull_mode(GS_NEITHER);
		while (gs_effect_loop(defaultEffect, "Draw")) {
			gs_effect_set_texture(gs_effect_get_param_by_name(defaultEffect,
				"image"), vidtex);
			gs_draw_sprite(vidtex, 0, width, height);
		}
	}

	return didMorph;
}

void Mask::MaskData::RewindAnimations() {

	// reset instance datas
	std::vector<std::shared_ptr<InstanceData>> idatas = instanceDatas.GetInstances();
	for (auto const& id : idatas) {
		id->Reset();
	}

	// reset our time
	m_elapsedTime = 0.0f;
}


