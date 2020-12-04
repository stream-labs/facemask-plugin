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

#include "mask-resource-effect.h"
#include "plugin/exceptions.h"
#include "plugin/plugin.h"
#include "plugin/utils.h"

extern "C" {
#include <libobs/graphics/graphics.h>
}

const std::map<std::string, std::string> Mask::Resource::Effect::g_textureTypes = {
			{"ambientTex", "AMBIENT_TEX"},
			{"diffuseTex", "DIFFUSE_TEX"},
			{"specularTex", "SPECULAR_TEX"},
			{"emissiveTex", "EMISSIVE_TEX"},
			{"normalTex", "NORMAL_TEX"},
			{"reflectTex", "REFLECT_TEX"},
			{"iblSpecTex", "IBL_SPEC_TEX"},
			{"iblDiffTex", "IBL_DIFF_TEX"},
			{"iblBRDFTex", "IBL_BRDF_TEX"},
			{"roughnessTex", "ROUGHNESS_TEX"},
			{"metalnessTex", "METALNESS_TEX"},
			{"metallicRoughnessTex", "METALLICROUGHNESS_TEX"},
			{"image","EFFECT_DEFAULT_IMAGE"}/*,
			{"vidLightingTex", "USE_VIDEO_LIGHTING"}*/
};


std::shared_ptr<GS::Effect> Mask::Resource::Effect::compile(std::string name, std::string filename, Cache *cache)
{
	char *file_string;
	file_string = os_quick_read_utf8_file(filename.c_str());
	if (!file_string) {
		blog(LOG_ERROR, "Could not load effect file '%s'", filename.c_str());
		return nullptr;
	}

	/*
		These #defines can be applied cleaner.
		However, it will require modifying libobs
		to support getting defines parameters for
		compiling textures as well. Currently, it just
		sets the parameter for DX to null.
		TODO Mod libobs, or just port to direct OpenGL

	Update:
		Using 1x1 empty textures it seems this could be avoided.
	    This is here in case we see performance cost,
		and have to revert to this way.
	////////////////////////////////////////////////////////////

	std::string dynamic_shader_string;
	std::string unique_name = name;

	bool requires_unique_name = (name == "PBR");

	for (const std::string &tex_type : active_textures) {
		if (g_textureTypes.find(tex_type) != g_textureTypes.end()) {
			dynamic_shader_string += "#define " + g_textureTypes.at(tex_type) + "\n";
		}
		if (requires_unique_name)
			unique_name += "_" + tex_type;
	}
	dynamic_shader_string += file_string;

	*/
	std::string code = file_string;
	bfree(file_string);
	return std::make_shared<GS::Effect>(code, name, cache);
}

Mask::Resource::Effect::Effect(Mask::MaskData* parent, std::string name, obs_data_t* data, Cache *cache)
	: IBase(parent, name) {
	const char* const S_DATA = "data";
	if (!obs_data_has_user_value(data, S_DATA)) {
		PLOG_ERROR("Effect '%s' has no data.", name.c_str());
		throw std::logic_error("Effect has no data.");
	}
	const char* base64data = obs_data_get_string(data, S_DATA);
	if (base64data[0] == '\0') {
		PLOG_ERROR("Effect '%s' has empty data.", name.c_str());
		throw std::logic_error("Effect has empty data.");
	}

	// write to temp file
	m_filename = Utils::Base64ToTempFile(base64data);
	m_filenameIsTemp = true;

	// cache
	m_cache = cache;
}

Mask::Resource::Effect::Effect(Mask::MaskData* parent, std::string name, std::string filename, Cache *cache)
	: IBase(parent, name) {

	m_filename = filename;
	m_filenameIsTemp = false;

	// cache
	m_cache = cache;
}


Mask::Resource::Effect::~Effect() {}

Mask::Resource::Type Mask::Resource::Effect::GetType() {
	return Mask::Resource::Type::Effect;
}

void Mask::Resource::Effect::Update(Mask::Part* part, float time) {
	UNUSED_PARAMETER(part);
	UNUSED_PARAMETER(time);
	return;
}

void Mask::Resource::Effect::Render(Mask::Part* part) {
	UNUSED_PARAMETER(part);
	if (m_Effect == nullptr) {

		m_Effect = Effect::compile(m_name, m_filename, m_cache);

		if (m_filenameIsTemp) {
			Utils::DeleteTempFile(m_filename);
		}
		m_filename.clear();
	}

	return;
}
