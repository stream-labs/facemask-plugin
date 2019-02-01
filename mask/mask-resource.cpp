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

#include <Windows.h>


Mask::Resource::IBase::IBase(Mask::MaskData* parent, std::string name) {
	m_parent = parent;
	m_name = name;
	std::hash<std::string> hasher;
	m_id = hasher(m_name);
}

Mask::Resource::IBase::~IBase() {

}

std::shared_ptr<Mask::Resource::IBase> Mask::Resource::IBase::Load(Mask::MaskData* parent, std::string name, obs_data_t* data) {
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
		return std::make_shared<Mask::Resource::Image>(parent, name, data);
	} 
	else if (type == "sequence") {
		// Image Sequence
		return std::make_shared<Mask::Resource::Sequence>(parent, name, data);
	}
	else if (type == "effect") {
		// Effect Shader
		return std::make_shared<Mask::Resource::Effect>(parent, name, data);
	} 
	else if (type == "material") {
		// Material (Combines Images, Effects
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
	else if (type == "sound") {
		// Sound (not supported)
		//return std::make_shared<Mask::Resource::Effect>(parent, name, data);
	}
	return nullptr;
}

std::shared_ptr<Mask::Resource::IBase> Mask::Resource::IBase::LoadDefault(Mask::MaskData* parent, std::string name) {
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


	static obs_data_t *g_templates;
	// load once
	if (g_templates == nullptr) {
		// load templates file
		char* f = obs_module_file("resources/templates.json");
		g_templates = obs_data_create_from_json_file(f);
	}

	// yield
	::Sleep(0);

	// image?
	if (g_defaultImages.find(name) != g_defaultImages.end()) {
		char* f = obs_module_file(g_defaultImages.at(name).c_str());
		p = std::make_shared<Mask::Resource::Image>
			(parent, name, std::string(f));
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
			(parent, name, std::string(f));
		bfree(f); 
	}
	// template?
	if (obs_data_has_user_value(g_templates, name.c_str())) {
		obs_data_t* template_data = obs_data_get_obj(g_templates, name.c_str());
		p = Load(parent, name, template_data);
		obs_data_release(template_data);
	}

	return p;
}

