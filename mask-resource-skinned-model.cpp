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
static const char* const S_POSITION = "position";
static const char* const S_ROTATION = "rotation";
static const char* const S_QROTATION = "qrotation";
static const char* const S_SCALE = "scale";



Mask::Resource::SkinnedModel::SkinnedModel(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name) {

	// cache hash ids for bones 
	std::hash<std::string> hasher;
	char temp[64];
	for (int i = 0; i < 8; i++) {
		snprintf(temp, sizeof(temp), "bone%d", i);
		m_boneIds[i] = hasher(temp);
	}

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
	int numBones = 0;
	for (obs_data_item_t* itm = obs_data_first(bonesData); itm; obs_data_item_next(&itm)) {
		numBones++;
	}

	m_bones.resize(numBones);
	for (obs_data_item_t* itm = obs_data_first(bonesData); itm; obs_data_item_next(&itm)) {
		obs_data_t* boneData = obs_data_item_get_obj(itm);

		// use string key as index into array
		std::string nn = obs_data_item_get_name(itm);
		int boneIndex = atoi(nn.c_str()); 
		Bone& bone = m_bones[boneIndex];

		if (!obs_data_has_user_value(boneData, S_NAME)) {
			PLOG_ERROR("Skinned Model '%s' bone has no name.", name.c_str());
			throw std::logic_error("Skinned Model has a bone with no name.");
		}

		std::string partName = obs_data_get_string(boneData, S_NAME);
		bone.part = parent->GetPart(partName);

		// Read offset transform data
		vec3 position, scale; 
		quat qrotation;
		vec3_zero(&position); 
		vec3_set(&scale, 1, 1, 1);
		quat_identity(&qrotation);
		if (obs_data_has_user_value(boneData, S_POSITION))
			obs_data_get_vec3(boneData, S_POSITION, &position);
		if (obs_data_has_user_value(boneData, S_QROTATION))
			obs_data_get_quat(boneData, S_QROTATION, &qrotation);
		if (obs_data_has_user_value(boneData, S_SCALE))
			obs_data_get_vec3(boneData, S_SCALE, &scale);

		// Calculate offset matrix 
		matrix4_identity(&bone.offset);
		matrix4_scale3f(&bone.offset, &bone.offset,
			scale.x, scale.y, scale.z);
		matrix4 qm;
		matrix4_from_quat(&qm, &qrotation);
		matrix4_mul(&bone.offset, &bone.offset, &qm);
		matrix4_translate3f(&bone.offset, &bone.offset,
			position.x, position.y, position.z);

		matrix4_identity(&bone.global);
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
		Mask::Resource::SkinnedModel::SkinnedModel::Skin skin;

		if (!obs_data_has_user_value(skinData, S_MESH)) {
			PLOG_ERROR("Skinned Model '%s' has skin with no mesh.", name.c_str());
			throw std::logic_error("Skinned Model has skin with no mesh.");
		}
		std::string meshName = obs_data_get_string(skinData, S_MESH);
		skin.mesh = std::dynamic_pointer_cast<Mesh>(m_parent->GetResource(meshName));
		if (skin.mesh == nullptr) {
			PLOG_ERROR("<Skinned Model '%s'> Dependency on mesh '%s' could not be resolved.",
				m_name.c_str(), meshName.c_str());
			throw std::logic_error("Model depends on non-existing mesh.");
		}
		if (skin.mesh->GetType() != Type::Mesh) {
			PLOG_ERROR("<Skinned Model '%s'> Resolved mesh dependency on '%s' is not a mesh.",
				m_name.c_str(), meshName.c_str());
			throw std::logic_error("Mesh dependency of Skinned Model is not a mesh.");
		}

		for (obs_data_item_t* itm2 = obs_data_first(skinBonesData); itm2; obs_data_item_next(&itm2)) {
			int boneIdx = (int)obs_data_item_get_int(itm2);
			skin.bones.push_back(boneIdx);
		}

		m_skins.emplace_back(skin);
	}
}

Mask::Resource::SkinnedModel::~SkinnedModel() {}

Mask::Resource::Type Mask::Resource::SkinnedModel::GetType() {
	return Mask::Resource::Type::SkinnedModel;
}

void Mask::Resource::SkinnedModel::Update(Mask::Part* part, float time) {
	part->mask->instanceDatas.Push(m_id);
	// update material & meshes
	m_material->Update(part, time);
	for (unsigned int i = 0; i < m_skins.size(); i++) {
		m_skins[i].mesh->Update(part, time);
	}
	// update bone matrices
	for (unsigned int i = 0; i < m_bones.size(); i++) {
		Bone& bone = m_bones[i];

		// concat offset and bone matrices
		matrix4_mul(&bone.global, &bone.offset, &bone.part->global);

		// need to transpose, since we are passing to a shader
		matrix4_transpose(&bone.global, &bone.global);
	}
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
	UNUSED_PARAMETER(part);

	// transform comes from bones, get rid of part transform
	// todo: mask could just not set transform if skinned mesh, maybe
	//       make base virtual method needsTransform?
	gs_matrix_pop();
	gs_matrix_push();

	part->mask->instanceDatas.Push(m_id);
	BonesList bone_list;
	for (unsigned int i = 0; i < m_skins.size(); i++) {
		const Skin& skin = m_skins[i];

		// set up bones list
		bone_list.numBones = (int)skin.bones.size();
		for (int j = 0; j < skin.bones.size(); j++) {
			bone_list.bones[j] = &(m_bones[skin.bones[j]].global);
		}
		// draw
		while (m_material->Loop(part, &bone_list)) {
			skin.mesh->Render(part);
		}
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
	// average all mesh centers to find center
	// TODO: optimize this. save it or something.
	vec4 c;
	vec4_zero(&c);
	int count = 0;
	for (auto skin : m_skins) {
		vec4 cc = skin.mesh->GetCenter();
		vec4_add(&c, &c, &cc);
		count++;
	}
	vec4_divf(&c, &c, (float)count);
	matrix4 m;
	gs_matrix_get(&m);
	vec4_transform(&c, &c, &m);
	return c.z;
}
	
void Mask::Resource::SkinnedModel::SortedRender() {
	DirectRender(sortDrawPart);
}

