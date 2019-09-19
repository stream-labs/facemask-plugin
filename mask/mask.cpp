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
#include "plugin/exceptions.h"
#include "plugin/strings.h"
#include "plugin/plugin.h"
#include "plugin/utils.h"

#include <set>

static float FOVA(float aspect) {
	// field of view angle matched to focal length for solvePNP
	return 56.0f / aspect;
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
static const char* const JSON_DRAW_VIDEO_WITH_MASK = "draw_video_with_mask";
static const char* const JSON_INTRO_FADE_TIME = "intro_fade_time";
static const char* const JSON_INTRO_DURATION = "intro_duration";


static const float BUCKETS_MAX_Z = 10.0f;
static const float BUCKETS_MIN_Z = -100.0f;

Mask::MaskData::MaskData(Cache *cache) : m_data(nullptr), m_morph(nullptr),
m_cache(cache), m_elapsedTime(0.0f) {
	ClearSortedDrawObjects();
	m_vidLightTex = nullptr;
	m_num_render_layers = 1;
	m_num_render_orders = 1;
}

Mask::MaskData::~MaskData() {
	Clear();
}

bool Mask::MaskData::NeedsPBRLighting() {
	for (auto &kv : m_resources) {
		if (kv.second->GetType() == Resource::Type::Material)
		{
			std::shared_ptr<Resource::Material> mat = std::dynamic_pointer_cast<Resource::Material>(kv.second);
			if (mat->IsPBR()) return true;
		}
	}
	return false;
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

	if (obs_data_has_user_value(m_data, JSON_DRAW_VIDEO_WITH_MASK))
		m_drawVideoWithMask = obs_data_get_bool(m_data, JSON_DRAW_VIDEO_WITH_MASK);
	else
		m_drawVideoWithMask = false;


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

	// yield
	::Sleep(0);

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
	obs_data_release(partData);
	if (partDataItem) {
		obs_data_item_release(&partDataItem);
	}

	// yield
	::Sleep(0);

	// Except for animations. 
	obs_data_t* resources = obs_data_get_obj(m_data, JSON_RESOURCES);
	for (obs_data_item_t* el = obs_data_first(resources); el; obs_data_item_next(&el)) {
		std::string resourceName = obs_data_item_get_name(el);
		obs_data_t* resd = obs_data_item_get_obj(el);
		if (!resd)
			continue;
		if (!obs_data_has_user_value(resd, JSON_TYPE)) {
			obs_data_release(resd);
			continue;
		}
		std::string resourceType = obs_data_get_string(resd, JSON_TYPE);
		if (resourceType == "animation") {
			m_animations.emplace(resourceName, 
				std::dynamic_pointer_cast<Resource::Animation>(GetResource(resourceName)));
		}
		obs_data_release(resd);
	}

	if (resources) {
		obs_data_release(resources);
	}

	// iterate resources and renumber render layer and order
	// this will help with both avoid illegal order numbers
	// and wasting space when layers/orders have gap between them
	using RenderObj = std::shared_ptr<SortedDrawObject>;
	auto comp_layer = [](RenderObj a, RenderObj b) {
		return a->m_render_layer < b->m_render_layer;
	};
	auto comp_order = [](RenderObj a, RenderObj b) {
		return (a->m_render_layer < b->m_render_layer) ||
			(a->m_render_layer == b->m_render_layer && (a->m_render_order < b->m_render_order));
	};
	std::multiset<RenderObj, decltype(comp_layer)> layer_set(comp_layer);
	std::multiset<RenderObj, decltype(comp_order)> order_set(comp_order);
	for (const auto &kv : m_resources) {
		std::shared_ptr<SortedDrawObject> model = std::dynamic_pointer_cast<SortedDrawObject>(kv.second);
		if (model)
		{
			layer_set.insert(model);
			order_set.insert(model);
		}
	}

	int current_layer = -1;
	int last_render_layer = -1;
	for (auto it = layer_set.begin(); it != layer_set.end(); it++)
	{
		if (current_layer == -1 || (*it)->m_render_layer > last_render_layer)
			current_layer++;

		last_render_layer = (*it)->m_render_layer;
		(*it)->m_render_layer = current_layer;
	}
	m_num_render_layers = current_layer + 1;

	int current_order = -1;
	int last_render_order = -1;
	last_render_layer = -1;
	for (auto it = order_set.begin(); it != order_set.end(); it++)
	{
		if (current_order == -1 ||
			(*it)->m_render_order > last_render_order ||
			(*it)->m_render_layer > last_render_layer)
			current_order++;

		last_render_order = (*it)->m_render_order;
		last_render_layer = (*it)->m_render_layer;
		(*it)->m_render_order = current_order;
	}
	m_num_render_orders = current_order + 1;

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

std::shared_ptr<Mask::Resource::IBase> Mask::MaskData::GetResource(const std::string& name, bool force_reload) {
	std::string res_name = name;
	if (res_name.length() == 0)
		throw std::invalid_argument("res_name must be at least one character long");

	auto kv = m_resources.find(res_name);
	if (!force_reload)
	{
		if (kv != m_resources.end())
			return kv->second;
	}
	else
	{
		// if force reload, create unique res_name
		if (kv != m_resources.end())
		{
			int i = 0;
			std::string new_string = res_name + "-";
			while (m_resources.find(new_string + std::to_string(i)) != m_resources.end())
				i++;
			res_name = new_string + std::to_string(i);
		}
	}

	// Default Resources
	std::shared_ptr<Mask::Resource::IBase> p = Resource::IBase::LoadDefault(this, name, GetCache());
	if (p) {
		/*
		// if resource is an effect, change to name to include active #defines
		auto effect = std::dynamic_pointer_cast<Mask::Resource::Effect>(p);
		if (effect) {
			// we have sorted texture vector. so for the same type of textures
			// this will always create one unique name
			for (const auto &t : effect->GetActiveTextures())
				res_name += "_" + t;
		}
		*/
		this->AddResource(res_name, p);
		return p;
	}

	// yield
	::Sleep(0);

	// Lazy Loaded
	obs_data_item_t* resources = obs_data_item_byname(m_data, JSON_RESOURCES);
	if (!resources) return nullptr;
	if (obs_data_item_gettype(resources) != OBS_DATA_OBJECT) {
		obs_data_item_release(&resources);
		return nullptr;
	}
	obs_data_t* resd = obs_data_item_get_obj(resources);
	if (!resd) {
		obs_data_item_release(&resources);
		return nullptr;
	}
	obs_data_item_t* element = obs_data_item_byname(resd, res_name.c_str());
	if (!element) {
		obs_data_item_release(&resources);
		obs_data_release(resd);
		return nullptr;
	}
	if (obs_data_item_gettype(element) != OBS_DATA_OBJECT) return nullptr;
	obs_data_t* elmd = obs_data_item_get_obj(element);
	if (!elmd) {
		obs_data_item_release(&element);
		obs_data_item_release(&resources);
		obs_data_release(resd);
		return nullptr;
	}
	try {
		auto res = Resource::IBase::Load(this, res_name, elmd, GetCache());
		if (res) {
			this->AddResource(res_name, res);
			obs_data_item_release(&element);
			obs_data_item_release(&resources);
			obs_data_release(resd);
			obs_data_release(elmd);
			return res;
		}
	} catch (...) {
		PLOG_DEBUG("Resource %s has THROWN AN EXCEPTION. MASK DID NOT LOAD CORRECTLY.", res_name.c_str());
		obs_data_item_release(&element);
		obs_data_item_release(&resources);
		obs_data_release(resd);
		obs_data_release(elmd);
		return nullptr;
	}
	obs_data_item_release(&element);
	obs_data_item_release(&resources);
	obs_data_release(resd);
	obs_data_release(elmd);
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
	part->name = name;
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
	if (!parts) {
		return nullptr;
	}
	if (obs_data_item_gettype(parts) != OBS_DATA_OBJECT) {
		obs_data_item_release(&parts);
		return nullptr;
	}
	obs_data_t* resd = obs_data_item_get_obj(parts);
	if (!resd) {
		obs_data_item_release(&parts);
		return nullptr;
	}
	obs_data_item_t* element = obs_data_item_byname(resd, name.c_str());
	if (!element) {
		obs_data_item_release(&parts);
		obs_data_release(resd);
		return nullptr;
	}
	if (obs_data_item_gettype(element) != OBS_DATA_OBJECT) {
		obs_data_item_release(&parts);
		obs_data_item_release(&element);
		obs_data_release(resd);
		return nullptr;
	}
	obs_data_t* elmd = obs_data_item_get_obj(element);
	if (!elmd) {
		obs_data_item_release(&parts);
		obs_data_item_release(&element);
		obs_data_release(resd);
		return nullptr;
	}
	std::shared_ptr<Mask::Part> res = LoadPart(name, elmd);
	obs_data_item_release(&parts);
	obs_data_item_release(&element);
	obs_data_release(resd);
	obs_data_release(elmd);
	return res;
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

size_t Mask::MaskData::GetNumParts() { 
	return m_parts.size(); 
}

std::shared_ptr<Mask::Part> Mask::MaskData::GetPart(int index) {
	int count = 0;
	std::shared_ptr<Mask::Part> el = nullptr;
	std::map<std::string, std::shared_ptr<Part>>::iterator kv = m_parts.begin();
	while (kv != m_parts.end()) {
		if (count == index)
			el = kv->second;
		count++;
		kv++;
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
	float z = obj->SortDepth() + obj->m_depth_bias;
	float normalized_z = z;
	if (z > BUCKETS_MAX_Z)
		normalized_z = BUCKETS_MAX_Z;
	if (z < BUCKETS_MIN_Z)
		normalized_z = BUCKETS_MIN_Z;
	normalized_z = (normalized_z - BUCKETS_MIN_Z) / (BUCKETS_MAX_Z - BUCKETS_MIN_Z);
	int idx = (int)(normalized_z * (float)(NUM_DRAW_BUCKETS - 1));

	SortedDrawObject **node = &m_drawBuckets[idx];
	while (*node) {
		if (*node == obj)
		{
			blog(LOG_ERROR, "Duplicate SortedDrawObject was found. Discarding...");
			return;
		}

		if (z < ((*node)->SortDepth() + (*node)->m_depth_bias)) break;
		node = &(*node)->nextDrawObject;
	}
	obj->nextDrawObject = *node;
	*node = obj;

	obj->instanceId = instanceDatas.CurrentId();

}

void Mask::MaskData::Decompose(const matrix4 *src, vec3 *s, matrix4 *R, vec3 *t)
{
	// Decompose the matrix fully
	// in case of negative scalings
	// the exact scale axis is not
	// known anymore, and we just pick
	// x-axis as the best we can.
	// See the following links for more information:
	//  - Last Paragraph:
	//    http://callumhay.blogspot.com/2010/10/decomposing-affine-transforms.html
	//  - Three.js PR:
	//    https://github.com/mrdoob/three.js/pull/4272

	float sx = vec4_len(&src->x);
	float sy = vec4_len(&src->y);
	float sz = vec4_len(&src->z);

	if (matrix4_determinant(src) < 0)
		sx = -sx;

	vec3_set(s, sx, sy, sz);
	vec3_from_vec4(t, &src->t);

	vec3 inv_s;
	vec3_set(&inv_s, 1/sx, 1/sy, 1/sz);

	matrix4_scale_i(R, &inv_s, src);
	vec4_set(&(R->t), 0, 0, 0, 1.0);

}

void  Mask::MaskData::PartCalcMatrix(Part *part) {

	// Only if we're dirty
	if (part->dirty) {
		Mask::Part* current_part = part;

		// pre-check dirty status for the local chain
		// we check if *any* of local transform nodes are dirty
		// if there's one, we will recalculate the local matrix
		bool local_chain_dirty = true;
		Mask::Part* temp_part = part;
		do {
			if (temp_part->localdirty) {
				local_chain_dirty = true;
				break;
			}
			temp_part = temp_part->parent.get();
		}
		while (temp_part && temp_part->local_to.length() > 0 && temp_part->local_to == part->name);


		if (local_chain_dirty) {
			do {
				// Calculate local matrix
				matrix4_identity(&current_part->local);

				matrix4_scale3f(&current_part->local, &current_part->local,
					current_part->scale.x, current_part->scale.y, current_part->scale.z);
				if (current_part->isquat) {
					matrix4 qm;
					matrix4_from_quat(&qm, &current_part->qrotation);
					matrix4_mul(&current_part->local, &current_part->local, &qm);
				}
				else {
					matrix4_rotate_aa4f(&current_part->local, &current_part->local,
						1.0f, 0.0f, 0.0f, current_part->rotation.x);
					matrix4_rotate_aa4f(&current_part->local, &current_part->local,
						0.0f, 1.0f, 0.0f, current_part->rotation.y);
					matrix4_rotate_aa4f(&current_part->local, &current_part->local,
						0.0f, 0.0f, 1.0f, current_part->rotation.z);
				}
				matrix4_translate3f(&current_part->local, &current_part->local,
					current_part->position.x, current_part->position.y, current_part->position.z);

				current_part->localdirty = false;

				// a local child node?
				if (current_part != part)
					matrix4_mul(&part->local, &part->local, &current_part->local);

				current_part = current_part->parent.get();

			} while (current_part && current_part->local_to.length() > 0 && current_part->local_to == part->name);
		}
		// Calculate global
		matrix4_copy(&part->global, &part->local);
		if (current_part) {
			PartCalcMatrix(current_part);
			if (part->inherit_type == Part::Inherit_RSrs) {
				matrix4_mul(&part->global, &part->global, &current_part->global);
			}
			else {
				/*
					The other two types of inherit types are more complex, and need
					several matrix decompositions. The details of the process can
					be seen in FBX SDK example here:
					http://help.autodesk.com/cloudhelp/2018/ENU/FBX-Developer-Help/cpp_ref/_transformations_2main_8cxx-example.html

					Naming reference:
					L: Local
					G: Global
					P: Parent
					M: Matrix
					v: Vector
					R: rotation
					T: translation
				*/

				// Local Matrix
				const matrix4 &LM = part->local;
				vec3 ls_v, lt_v;
				matrix4 LR, LS;
				Decompose(&LM, &ls_v, &LR, &lt_v);

				matrix4_identity(&LS);
				matrix4_scale(&LS, &LS, &ls_v);

				// Parent Global Matrix
				const matrix4 &PGM = current_part->global;
				vec3 pgs_v, pgt_v;
				matrix4 PGR;
				Decompose(&PGM, &pgs_v, &PGR, &pgt_v);

				// pgs_v will have scale information
				// if we need to have shear information as well
				// we'll have to find PGS matrix using the following:
				// PGS = PGM * inv_PGT * inv_PGR
				matrix4 PGS;
				matrix4_identity(&PGS);
				matrix4_scale(&PGS, &PGS, &pgs_v);

				// Global Rotation x Scale Matrix
				matrix4 GSR;
				matrix4_identity(&GSR);

				if (part->inherit_type == Part::Inherit_Rrs) {
					
					// Parent Local Matrix
					const matrix4 &PLM = current_part->local;
					vec3 pls_v, plt_v;
					matrix4 PLR;
					Decompose(&PLM, &pls_v, &PLR, &plt_v);

					matrix4 PGS_nolocal;
					matrix4_scale3f(&PGS_nolocal, &PGS, 1 / pls_v.x, 1 / pls_v.y, 1 / pls_v.z);

					// GSR = LS x PGS_nolocal x LR x PGR
					matrix4_mul(&GSR, &GSR, &LS);
					matrix4_mul(&GSR, &GSR, &PGS_nolocal);
					matrix4_mul(&GSR, &GSR, &LR);
					matrix4_mul(&GSR, &GSR, &PGR);
				}
				else if (part->inherit_type == Part::Inherit_RrSs) {
					// GSR = LS x PGS x LR x PGR
					matrix4_mul(&GSR, &GSR, &LS);
					matrix4_mul(&GSR, &GSR, &PGS);
					matrix4_mul(&GSR, &GSR, &LR);
					matrix4_mul(&GSR, &GSR, &PGR);
				}

				vec3 gt_v;
				matrix4 GT;
				vec3_transform(&gt_v, &lt_v, &PGM);

				matrix4_identity(&GT);
				matrix4_translate3v(&GT, &GT, &gt_v);

				matrix4_mul(&part->global, &GSR, &GT);

			}
		}
		part->dirty = false;
	}
}

void Mask::MaskData::Tick(float time) {
	// update animations with the first Part
	Part* p = nullptr;
	for (auto aakv : m_animations) {
		if (aakv.second) {
			if (!p) {
				for (auto kv : m_parts) {
					p = kv.second.get();
					break;
				}
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
		if (kv.second->local_to.length() == 0)
		{
			PartCalcMatrix(kv.second.get());
		}
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
	
	// DO INTRO ANIMATION FADING
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

void Mask::MaskData::SetTransform(const smll::ThreeDPose& pose, bool billboard)
{
	gs_matrix_identity();
	gs_matrix_translate3f((float)pose.translation[0],
		(float)pose.translation[1], (float)-pose.translation[2]);
	if (!billboard) {
		gs_matrix_rotaa4f((float)pose.rotation[0], (float)pose.rotation[1],
			(float)-pose.rotation[2], (float)-pose.rotation[3]);
	}
}

void Mask::MaskData::Render(const smll::DetectionResults &faces, int width, int height, bool depthOnly) {

	gs_viewport_push();
	gs_projection_push();

	gs_set_viewport(0, 0, width, height);
	gs_enable_depth_test(true);
	gs_depth_function(GS_GREATER);

	float aspect = (float)width / (float)height;
	// using reversed-z depth with infinite far
	gs_perspective(FOVA(aspect), aspect, 1.0, 0.0);
	
	ClearSortedDrawObjects();

	gs_blend_state_push();
	gs_reset_blend_state();
	gs_enable_blending(true);
	gs_blend_function(gs_blend_type::GS_BLEND_ONE,
		gs_blend_type::GS_BLEND_ZERO);
	gs_enable_color(true, true, true, true);

	// save a matrix for face transforms
	gs_matrix_push();

	// OPAQUE
	for (auto kv : m_parts) {
		if (kv.second->resources.size() == 0)
			continue;
		instanceDatas.Push(kv.second->hash_id);
		for (auto  it = kv.second->resources.begin();
			it != kv.second->resources.end(); it++) {

			if ((*it)->IsDepthOnly() != depthOnly) continue;

			bool billboard = (*it)->IsRotationDisabled();

			for (int i = 0; i < faces.length; i++) {
				// NOTE for some reason, some masks
				// have their depth head set to static
				if ((*it)->IsStatic() && (*it)->IsDepthOnly() == false)
					SetTransform(faces[i].startPose, billboard);
				else
					SetTransform(faces[i].pose, billboard);

				gs_matrix_push();
				gs_matrix_mul(&kv.second->global);

				(*it)->Render(kv.second.get());

				gs_matrix_pop();
			}
		}
		instanceDatas.Pop();
	}

	// TRANSPARENT
	gs_blend_function_separate(gs_blend_type::GS_BLEND_SRCALPHA,
		gs_blend_type::GS_BLEND_INVSRCALPHA, gs_blend_type::GS_BLEND_ONE, gs_blend_type::GS_BLEND_INVSRCALPHA);

	for (size_t current_order = 0; current_order < m_num_render_orders; current_order++)
	{
		for (unsigned int i = 0; i < NUM_DRAW_BUCKETS; i++) {
			SortedDrawObject* head_sdo = m_drawBuckets[i];
			SortedDrawObject* sdo = head_sdo;
			Resource::IBase* res = dynamic_cast<Resource::IBase*>(sdo);
			if (!res) continue; // skip if sdo is not a resource type
			while (sdo) {
				if (sdo->m_render_order == current_order)
				{
					Part* part = sdo->sortDrawPart;
					instanceDatas.PushDirect(sdo->instanceId);
					
					bool billboard = res->IsRotationDisabled();

					for (int i = 0; i < faces.length; i++) {
						if (res->IsStatic())
							SetTransform(faces[i].startPose, billboard);
						else
							SetTransform(faces[i].pose, billboard);

						gs_matrix_push();
						gs_matrix_mul(&part->global);
						sdo->SortedRender();
						gs_matrix_pop();
					}
					instanceDatas.Pop();
				}
				sdo = sdo->nextDrawObject;
				if (sdo == head_sdo) {
					blog(LOG_ERROR, "Loop found in SortedDrawObject list. Looped back to '%s'. Breaking the loop...", res->GetName().c_str());
					break;
				}
			}
		}
	}

	gs_matrix_pop();

	gs_blend_state_pop();

	gs_projection_pop();
	gs_viewport_pop();

}

static const char* const S_POSITION = "position";
static const char* const S_ROTATION = "rotation";
static const char* const S_QROTATION = "qrotation";
static const char* const S_SCALE = "scale";
static const char* const S_LOCAL_TO = "local-to";
static const char* const S_INHERIT_TYPE = "inherit-type";
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

	if (obs_data_has_user_value(data, S_LOCAL_TO))
		current->local_to = obs_data_get_string(data, S_LOCAL_TO);

	if (obs_data_has_user_value(data, S_INHERIT_TYPE))
		current->inherit_type = static_cast<Part::InheritType>(obs_data_get_int(data, S_INHERIT_TYPE));
	
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
			if (rezItem) {
				obs_data_item_release(&rezItem);
			}
			return current;
		}
		for (obs_data_item_t* itm = obs_data_first(rezData); itm; obs_data_item_next(&itm)) {
			std::string resourceName = obs_data_item_get_string(itm);
			current->resources.push_back(GetResource(resourceName));
		}
		if (rezItem) {
			obs_data_item_release(&rezItem);
		}
		obs_data_release(rezData);
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

float	Mask::MaskData::GetGlobalAlpha() {
	std::shared_ptr<Mask::AlphaInstanceData> aid =
		instanceDatas.GetData<Mask::AlphaInstanceData>(Mask::AlphaInstanceDataId);
	return aid->alpha;
}

void	Mask::MaskData::SetGlobalAlpha(float alpha) {
	std::shared_ptr<Mask::AlphaInstanceData> aid =
		instanceDatas.GetData<Mask::AlphaInstanceData>(Mask::AlphaInstanceDataId);
	aid->alpha = alpha;
}


void	Mask::MaskData::Play() {
	for (auto aakv : m_animations) {
		if (aakv.second) {
			aakv.second->Play();
		}
	}
}

void	Mask::MaskData::PlayBackwards() {
	for (auto aakv : m_animations) {
		if (aakv.second) {
			aakv.second->PlayBackwards();
		}
	}
}

void	Mask::MaskData::Stop() {
	for (auto aakv : m_animations) {
		if (aakv.second) {
			aakv.second->Stop();
		}
	}
}

void	Mask::MaskData::Rewind(bool last) {
	for (auto aakv : m_animations) {
		if (aakv.second) {
			aakv.second->Rewind(last);
		}
	}
	// reset instance datas
	ResetInstanceDatas();
	// reset our time
	m_elapsedTime = 0.0f;
}

float	Mask::MaskData::GetDuration() {
	// find max
	float d = 0.0f;
	for (auto aakv : m_animations) {
		if (aakv.second) {
			if (d < aakv.second->GetDuration())
				d = aakv.second->GetDuration();
		}
	}
	return d;
}

float	Mask::MaskData::LastFrame() {
	// find max
	float f = 0.0f;
	for (auto aakv : m_animations) {
		if (aakv.second) {
			if (f < aakv.second->LastFrame())
				f = aakv.second->LastFrame();
		}
	}
	return f;
}

float	Mask::MaskData::GetFPS() {
	// just grab first
	for (auto aakv : m_animations) {
		if (aakv.second) {
			return aakv.second->GetFPS();
		}
	}
	return 0.0f;
}

float	Mask::MaskData::GetPlaybackSpeed() {
	// just grab first
	for (auto aakv : m_animations) {
		if (aakv.second) {
			return aakv.second->GetPlaybackSpeed();
		}
	}
	return 0.0f;
}

void	Mask::MaskData::SetPlaybackSpeed(float speed) {
	for (auto aakv : m_animations) {
		if (aakv.second) {
			aakv.second->SetPlaybackSpeed(speed);
		}
	}
}

void   Mask::MaskData::Seek(float time) {
	for (auto aakv : m_animations) {
		if (aakv.second) {
			aakv.second->Seek(time);
		}
	}
}

bool	Mask::MaskData::GetStopOnLastFrame() {
	// just grab first
	for (auto aakv : m_animations) {
		if (aakv.second) {
			return aakv.second->GetStopOnLastFrame();
		}
	}
	return false;
}

void	Mask::MaskData::SetStopOnLastFrame(bool stop) {
	for (auto aakv : m_animations) {
		if (aakv.second) {
			aakv.second->SetStopOnLastFrame(stop);
		}
	}
}

float	Mask::MaskData::GetPosition() {
	// just grab first
	for (auto aakv : m_animations) {
		if (aakv.second) {
			return aakv.second->GetPosition();
		}
	}
	return m_elapsedTime;
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

	std::shared_ptr<Mask::AlphaInstanceData> aid =
		instanceDatas.GetData<Mask::AlphaInstanceData>(Mask::AlphaInstanceDataId);

	// Add an empty morph resource if they want to use 
	// other features that depend on it
	if ((trires.autoBGRemoval || trires.cartoonMode) && m_morph == nullptr) {
		std::string n("auto_morph");
		std::shared_ptr<Mask::Resource::IBase> r = 
			std::make_shared<Mask::Resource::Morph>(this, n);
		AddResource(n, r);
	}

	if (m_morph && aid->alpha > 0.0f && trires.vertexBuffer) {
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

void Mask::MaskData::ResetInstanceDatas() {

	// reset instance datas
	std::vector<std::shared_ptr<InstanceData>> idatas = instanceDatas.GetInstances();
	for (auto id : idatas) {
		id->Reset();
	}
}


