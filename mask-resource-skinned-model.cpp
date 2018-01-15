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

#include "mask-resource-skinned-model.h"
#include "exceptions.h"
#include "plugin.h"
#include "utils.h"
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/util/platform.h>
	#include <libobs/obs-module.h>
	#pragma warning( pop )
}
#include <sstream>

static const char* const S_MATERIAL = "material";
static const char* const S_BONES = "bones";
static const char* const S_SKINS = "skins";
static const char* const S_NAME = "name";
static const char* const S_MESH = "mesh";
static const char* const S_OFFSET = "offset";
static const char* const S_MAT_A1 = "a1";
static const char* const S_MAT_A2 = "a2";
static const char* const S_MAT_A3 = "a3";
static const char* const S_MAT_A4 = "a4";
static const char* const S_MAT_B1 = "b1";
static const char* const S_MAT_B2 = "b2";
static const char* const S_MAT_B3 = "b3";
static const char* const S_MAT_B4 = "b4";
static const char* const S_MAT_C1 = "c1";
static const char* const S_MAT_C2 = "c2";
static const char* const S_MAT_C3 = "c3";
static const char* const S_MAT_C4 = "c4";
static const char* const S_MAT_D1 = "d1";
static const char* const S_MAT_D2 = "d2";
static const char* const S_MAT_D3 = "d3";
static const char* const S_MAT_D4 = "d4";



Mask::Resource::SkinnedModel::SkinnedModel(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name) {

	// Material
	if (!obs_data_has_user_value(data, S_MATERIAL)) {
		PLOG_ERROR("Skinned Model '%s' has no material.", name.c_str());
		throw std::logic_error("Skinned Model has no material.");
	}
	std::string materialName = obs_data_get_string(data, S_MATERIAL);
	m_material = std::dynamic_pointer_cast<Material>(m_parent->GetResource(materialName));
	if (m_material == nullptr) {
		PLOG_ERROR("<Skinned Model '%s'> Dependency on material '%s' could not be resolved.",
			m_name.c_str(), materialName.c_str());
		throw std::logic_error("Model depends on non-existing material.");
	}
	if (m_material->GetType() != Type::Material) {
		PLOG_ERROR("<Skinned Model '%s'> Resolved material dependency on '%s' is not a material.",
			m_name.c_str(), materialName.c_str());
		throw std::logic_error("Material dependency of Model is not a material.");
	}

	// Bones list
	if (!obs_data_has_user_value(data, S_BONES)) {
		PLOG_ERROR("Skinned Model '%s' has no bones list.", name.c_str());
		throw std::logic_error("Skinned Model has no bones.");
	}
	obs_data_item_t* bonesItem = obs_data_item_byname(data, S_BONES);
	if (obs_data_item_gettype(bonesItem) != obs_data_type::OBS_DATA_OBJECT) {
		PLOG_ERROR("Bad bones section in '%s'.", name.c_str());
		throw std::logic_error("Skinned Model has bad bones section.");
	}
	obs_data_t* bonesData = obs_data_item_get_obj(bonesItem);
	if (!bonesData) {
		PLOG_ERROR("Bad bones section in '%s'.", name.c_str());
		throw std::logic_error("Skinned Model has bad bones section.");
	}
	for (obs_data_item_t* itm = obs_data_first(bonesData); itm; obs_data_item_next(&itm)) {
		obs_data_t* boneData = obs_data_item_get_obj(itm);

		if (!obs_data_has_user_value(boneData, S_NAME)) {
			PLOG_ERROR("Skinned Model '%s' bone has no name.", name.c_str());
			throw std::logic_error("Skinned Model has a bone with no name.");
		}
		if (!obs_data_has_user_value(boneData, S_OFFSET)) {
			PLOG_ERROR("Skinned Model '%s' bone has no offset.", name.c_str());
			throw std::logic_error("Skinned Model has a bone with no offset.");
		}
		std::string boneName = obs_data_get_string(boneData, S_NAME);
		obs_data_t* offsetData = obs_data_get_obj(boneData, S_OFFSET);

		matrix4 m;
		m.x.x = (float)obs_data_get_double(offsetData, S_MAT_A1);
		m.x.y = (float)obs_data_get_double(offsetData, S_MAT_A2);
		m.x.z = (float)obs_data_get_double(offsetData, S_MAT_A3);
		m.x.w = (float)obs_data_get_double(offsetData, S_MAT_A4);
		m.y.x = (float)obs_data_get_double(offsetData, S_MAT_B1);
		m.y.y = (float)obs_data_get_double(offsetData, S_MAT_B2);
		m.y.z = (float)obs_data_get_double(offsetData, S_MAT_B3);
		m.y.w = (float)obs_data_get_double(offsetData, S_MAT_B4);
		m.z.x = (float)obs_data_get_double(offsetData, S_MAT_C1);
		m.z.y = (float)obs_data_get_double(offsetData, S_MAT_C2);
		m.z.z = (float)obs_data_get_double(offsetData, S_MAT_C3);
		m.z.w = (float)obs_data_get_double(offsetData, S_MAT_C4);
		m.t.x = (float)obs_data_get_double(offsetData, S_MAT_D1);
		m.t.y = (float)obs_data_get_double(offsetData, S_MAT_D2);
		m.t.z = (float)obs_data_get_double(offsetData, S_MAT_D3);
		m.t.w = (float)obs_data_get_double(offsetData, S_MAT_D4);
	}

	// Skins list
	if (!obs_data_has_user_value(data, S_SKINS)) {
		PLOG_ERROR("Skinned Model '%s' has no skins list.", name.c_str()); 
		throw std::logic_error("Skinned Model has no skins.");
	}
	obs_data_item_t* skinsItem = obs_data_item_byname(data, S_SKINS);
	if (obs_data_item_gettype(skinsItem) != obs_data_type::OBS_DATA_OBJECT) {
		PLOG_ERROR("Bad skins section in '%s'.", name.c_str());
		throw std::logic_error("Skinned Model has bad skins section.");
	}
	obs_data_t* skinsData = obs_data_item_get_obj(skinsItem);
	if (!skinsData) {
		PLOG_ERROR("Bad skins section in '%s'.", name.c_str());
		throw std::logic_error("Skinned Model has bad skins section.");
	}
	for (obs_data_item_t* itm = obs_data_first(skinsData); itm; obs_data_item_next(&itm)) {
		obs_data_t* skinData = obs_data_item_get_obj(itm);
		std::string skinName = obs_data_item_get_name(itm);
		obs_data_item_t* skinBonesItem = obs_data_item_byname(skinData, S_BONES);
		if (obs_data_item_gettype(skinBonesItem) != obs_data_type::OBS_DATA_OBJECT) {
			PLOG_ERROR("Bad bones section in skin in '%s'.", name.c_str());
			throw std::logic_error("Skinned Model has bad bones section in skin.");
		}
		obs_data_t* skinBonesData = obs_data_item_get_obj(skinBonesItem);
		if (!skinBonesData) {
			PLOG_ERROR("Bad bones section in skin in '%s'.", name.c_str());
			throw std::logic_error("Skinned Model has bad bones section in skin.");
		}
		std::string meshName = obs_data_get_string(skinData, S_MESH);
		for (obs_data_item_t* itm2 = obs_data_first(skinBonesData); itm2; obs_data_item_next(&itm2)) {
			int boneIdx = (int)obs_data_item_get_int(itm2);
		}
	}


}

Mask::Resource::SkinnedModel::~SkinnedModel() {}

Mask::Resource::Type Mask::Resource::SkinnedModel::GetType() {
	return Mask::Resource::Type::Model;
}

void Mask::Resource::SkinnedModel::Update(Mask::Part* part, float time) {
	part->mask->instanceDatas.Push(m_id);
	m_material->Update(part, time);
	m_mesh->Update(part, time);
	part->mask->instanceDatas.Pop();
}

void Mask::Resource::SkinnedModel::Render(Mask::Part* part) {
	// Add transparent models as sorted draw objects
	// to draw in a sorted second render pass 
	if (!IsOpaque()) {
		this->sortDrawPart = part;
		part->mask->AddSortedDrawObject(this);
		return;
	}
	DirectRender(part);
}

void Mask::Resource::SkinnedModel::DirectRender(Mask::Part* part) {
	part->mask->instanceDatas.Push(m_id);
	while (m_material->Loop(part)) {
		m_mesh->Render(part);
	}
	part->mask->instanceDatas.Pop();
}


bool Mask::Resource::SkinnedModel::IsDepthOnly() {
	if (m_material != nullptr) {
		return m_material->IsDepthOnly();
	}
	return false;
}

bool Mask::Resource::SkinnedModel::IsOpaque() {
	// note: depth only objects are considered opaque
	if (IsDepthOnly()) {
		return true;
	}
	if (m_material != nullptr) {
		return m_material->IsOpaque();
	}
	return true;
}

float Mask::Resource::SkinnedModel::SortDepth() {
	vec4 c = m_mesh->GetCenter();
	matrix4 m;
	gs_matrix_get(&m);
	vec4_transform(&c, &c, &m);
	return c.z;
}
	
void Mask::Resource::SkinnedModel::SortedRender() {
	sortDrawPart->mask->instanceDatas.Push(m_id);
	while (m_material->Loop(sortDrawPart)) {
		m_mesh->Render(sortDrawPart);
	}
	sortDrawPart->mask->instanceDatas.Pop();
}

