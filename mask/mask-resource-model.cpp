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

#include "mask-resource-model.h"
#include "plugin/exceptions.h"
#include "plugin/plugin.h"
#include "plugin/utils.h"
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/util/platform.h>
	#include <libobs/obs-module.h>
	#pragma warning( pop )
}
#include <sstream>

static const char* const S_MESH = "mesh";
static const char* const S_MATERIAL = "material";


Mask::Resource::Model::Model(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name) {
	if (!obs_data_has_user_value(data, S_MESH)) {
		PLOG_ERROR("Model '%s' has no mesh.", name.c_str());
		throw std::logic_error("Model has no mesh.");
	}
	std::string meshName = obs_data_get_string(data, S_MESH);
	m_mesh = std::dynamic_pointer_cast<Mesh>(m_parent->GetResource(meshName));
	if (m_mesh == nullptr) {
		PLOG_ERROR("<Model '%s'> Dependency on mesh '%s' could not be resolved.",
			m_name.c_str(), meshName.c_str());
		throw std::logic_error("Model depends on non-existing mesh.");
	}
	if (m_mesh->GetType() != Type::Mesh) {
		PLOG_ERROR("<Model '%s'> Resolved mesh dependency on '%s' is not a mesh.",
			m_name.c_str(), meshName.c_str());
		throw std::logic_error("Mesh dependency of Model is not a mesh.");
	}

	if (!obs_data_has_user_value(data, S_MATERIAL)) {
		PLOG_ERROR("Model '%s' has no material.", name.c_str());
		throw std::logic_error("Model has no material.");
	}
	std::string materialName = obs_data_get_string(data, S_MATERIAL);
	m_material = std::dynamic_pointer_cast<Material>(m_parent->GetResource(materialName));
	if (m_material == nullptr) {
		PLOG_ERROR("<Model '%s'> Dependency on material '%s' could not be resolved.",
			m_name.c_str(), materialName.c_str());
		throw std::logic_error("Model depends on non-existing material.");
	}
	if (m_material->GetType() != Type::Material) {
		PLOG_ERROR("<Model '%s'> Resolved material dependency on '%s' is not a material.",
			m_name.c_str(), materialName.c_str());
		throw std::logic_error("Material dependency of Model is not a material.");
	}
}

Mask::Resource::Model::~Model() {}

Mask::Resource::Type Mask::Resource::Model::GetType() {
	return Mask::Resource::Type::Model;
}

void Mask::Resource::Model::Update(Mask::Part* part, float time) {
	m_parent->instanceDatas.Push(m_id);
	m_material->Update(part, time);
	m_mesh->Update(part, time);
	m_parent->instanceDatas.Pop();
}

void Mask::Resource::Model::Render(Mask::Part* part) {
	// Add transparent models as sorted draw objects
	// to draw in a sorted second render pass 
	if (!IsOpaque()) {
		this->sortDrawPart = part;
		m_parent->AddSortedDrawObject(this);
		return;
	}
	DirectRender(part);
}

void Mask::Resource::Model::DirectRender(Mask::Part* part) {
	if (m_material->WriteAlpha())
		gs_enable_color(true, true, true, true);
	else
		gs_enable_color(true, true, true, false);

	m_parent->instanceDatas.Push(m_id);
	while (m_material->Loop(part)) {
		m_mesh->Render(part);
	}
	m_parent->instanceDatas.Pop();
}


bool Mask::Resource::Model::IsDepthOnly() {
	if (m_material != nullptr) {
		return m_material->IsDepthOnly();
	}
	return false;
}

bool Mask::Resource::Model::IsStatic() {
	if (m_material != nullptr) {
		return m_material->IsStatic();
	}
	return false;
}

bool Mask::Resource::Model::IsOpaque() {
	// note: depth only objects are considered opaque
	if (IsDepthOnly()) {
		return true;
	}
	if (m_material != nullptr) {
		return m_material->IsOpaque();
	}
	return true;
}

float Mask::Resource::Model::SortDepth() {
	vec4 c = m_mesh->GetCenter();
	matrix4 m;
	gs_matrix_get(&m);
	vec4_transform(&c, &c, &m);
	return c.z;
}
	
void Mask::Resource::Model::SortedRender() {
	DirectRender(sortDrawPart);
}

