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

#include "mask-resource-material.h"
#include "mask-resource-skinned-model.h"
#include "mask-resource-light.h"
#include "mask-resource-sequence.h"
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

static const char* const S_EFFECT = "effect";
static const char* const S_TECHNIQUE = "technique";
static const char* const S_PARAMETERS = "parameters";
static const char* const S_UWRAP = "u-wrap";
static const char* const S_VWRAP = "v-wrap";
static const char* const S_WWRAP = "w-wrap";
static const char* const S_CULLING = "culling";
static const char* const S_DEPTHTEST = "depth-test";
static const char* const S_DEPTHONLY = "depth-only";
static const char* const S_STATIC = "static";
static const char* const S_DISABLE_ROTATION = "disableRotation";
static const char* const S_OPAQUE = "opaque";
static const char* const S_FILTER = "filter";
static const char* const S_ALPHAWRITE = "alpha-write";


static const char* const PARAM_WORLD = "World";
static const char* const PARAM_TEXMAT = "TexMat";
static const char* const PARAM_ALPHA = "alpha";
static const char* const PARAM_NUMBONES = "numBones";
static const char* const PARAM_NUMLIGHTS = "numLights";

#undef GetObject

Mask::Resource::Material::Material(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name), m_effect(nullptr), m_looping(false), m_currentTechnique(nullptr),
	m_samplerState(nullptr), m_depthOnly(false), m_static(false), m_opaque(true), m_alphaWrite(true), m_rotationDisable(false) {
	
	std::hash<std::string> hasher;
	char temp[64];
	for (int i = 0; i < 8; i++) {
		snprintf(temp, sizeof(temp), "light%d", i);
		m_lightIds[i] = hasher(temp);
	}

	if (!obs_data_has_user_value(data, S_EFFECT)) {
		PLOG_ERROR("Material '%s' has no effect.", name.c_str());
		throw std::logic_error("Material has no effect.");
	}
	std::string effectName = obs_data_get_string(data, S_EFFECT);
	m_effect = std::dynamic_pointer_cast<Mask::Resource::Effect>(m_parent->GetResource(effectName));
	if (m_effect == nullptr) {
		PLOG_ERROR("Material uses unknown effect:", effectName.c_str());
		throw std::logic_error("Material uses non-existing effect.");
	} 

	if (obs_data_has_user_value(data, S_TECHNIQUE)) {
		m_technique = obs_data_get_string(data, S_TECHNIQUE);
	} else {
		m_technique = "Draw"; // Default to using draw.
	}

	m_wrapU = GS_ADDRESS_CLAMP;
	m_wrapV = GS_ADDRESS_CLAMP;
	m_wrapW = GS_ADDRESS_CLAMP;
	if (obs_data_has_user_value(data, S_UWRAP)) {
		m_wrapU = StringToAddressMode(obs_data_get_string(data, S_UWRAP));
	}
	if (obs_data_has_user_value(data, S_VWRAP)) {
		m_wrapV = StringToAddressMode(obs_data_get_string(data, S_VWRAP));
	}
	if (obs_data_has_user_value(data, S_WWRAP)) {
		m_wrapW = StringToAddressMode(obs_data_get_string(data, S_WWRAP));
	}
	m_culling = gs_cull_mode::GS_BACK;
	if (obs_data_has_user_value(data, S_CULLING)) {
		std::string culling = obs_data_get_string(data, S_CULLING);
		if (culling == "back")
			m_culling = gs_cull_mode::GS_BACK;
		else if (culling == "front")
			m_culling = gs_cull_mode::GS_FRONT;
		else if (culling == "neither")
			m_culling = gs_cull_mode::GS_NEITHER;
	}
	m_depthTest = gs_depth_test::GS_LESS;
	if (obs_data_has_user_value(data, S_DEPTHTEST)) {
		std::string test = obs_data_get_string(data, S_DEPTHTEST);
		if (test == "never")
			m_depthTest = gs_depth_test::GS_NEVER;
		else if (test == "less")
			m_depthTest = gs_depth_test::GS_LESS;
		else if (test == "less-equal")
			m_depthTest = gs_depth_test::GS_LEQUAL;
		else if (test == "equal")
			m_depthTest = gs_depth_test::GS_EQUAL;
		else if (test == "greater-equal")
			m_depthTest = gs_depth_test::GS_GEQUAL;
		else if (test == "greater")
			m_depthTest = gs_depth_test::GS_GREATER;
		else if (test == "not-equal")
			m_depthTest = gs_depth_test::GS_NOTEQUAL;
		else if (test == "always")
			m_depthTest = gs_depth_test::GS_ALWAYS;
	}
	m_depthOnly = false;
	if (obs_data_has_user_value(data, S_DEPTHONLY)) {
		m_depthOnly = obs_data_get_bool(data, S_DEPTHONLY);
	}
	m_static = false;
	if (obs_data_has_user_value(data, S_STATIC)) {
		m_static = obs_data_get_bool(data, S_STATIC);
	}
	m_rotationDisable = false;
	if (obs_data_has_user_value(data, S_DISABLE_ROTATION)) {
		m_rotationDisable = obs_data_get_bool(data, S_DISABLE_ROTATION);
	}
	m_opaque = true;
	if (obs_data_has_user_value(data, S_OPAQUE)) {
		m_opaque = obs_data_get_bool(data, S_OPAQUE);
	}
	m_alphaWrite = true;
	if (obs_data_has_user_value(data, S_ALPHAWRITE)) {
		m_alphaWrite = obs_data_get_bool(data, S_ALPHAWRITE);
	}
	m_filter = GS_FILTER_ANISOTROPIC;
	if (obs_data_has_user_value(data, S_FILTER)) {
		std::string filter = obs_data_get_string(data, S_FILTER);
		if (filter == "point")
			m_filter = GS_FILTER_POINT;
		else if (filter == "linear")
			m_filter = GS_FILTER_LINEAR;
		else if (filter == "anisotropic")
			m_filter = GS_FILTER_ANISOTROPIC;
		else if (filter == "min-mag-point-mip-linear")
			m_filter = GS_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		else if (filter == "min-point-mag-linear-mip-point")
			m_filter = GS_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		else if (filter == "min-point-mag-mip-linear")
			m_filter = GS_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		else if (filter == "min-linear-mag-mip-point")
			m_filter = GS_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		else if (filter == "min-linear-mag-point-mip-linear")
			m_filter = GS_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		else if (filter == "min-mag-linear-mip-point")
			m_filter = GS_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	}

	if (obs_data_has_user_value(data, S_PARAMETERS)) {
		obs_data_t* prmd = obs_data_get_obj(data, S_PARAMETERS);
		if (!prmd)
			return;
		// iterate parameters
		for (obs_data_item_t* el = obs_data_first(prmd); el;  obs_data_item_next(&el)) {
			std::string parameterName = obs_data_item_get_name(el);
			if (parameterName.length() == 0) {
				PLOG_WARNING("<Material '%s'> Parameter with no name found.",
					m_name.c_str());
				continue;
			}

			obs_data_t* eld = obs_data_item_get_obj(el);
			if (!eld)
				continue;
			if (!obs_data_has_user_value(eld, "type")|| !obs_data_has_user_value(eld, "value")) {
				obs_data_release(eld);
				continue;
			}

			std::string type = obs_data_get_string(eld, "type");
			if (type == "texture" || type == "image" || type == "sequence") {
				std::string value = obs_data_get_string(eld, "value");
				auto resource = m_parent->GetResource(value);
				if (resource != nullptr) {
					m_imageParameters.emplace(parameterName, resource);
				} else {
					PLOG_WARNING("<Material '%s'> Parameter '%s' uses non-existing Image '%s'.",
						m_name.c_str(), parameterName.c_str(), value.c_str());
				}
			} else if (type == "integer") {
				Parameter param;
				param.type = GS::EffectParameter::Type::Integer;
				param.intValue = (int32_t)obs_data_get_int(eld, "value");
				m_parameters.emplace(parameterName, param);
			} else if (type == "integer2") {
				Parameter param;
				param.type = GS::EffectParameter::Type::Integer2;
				obs_data_t* vector = obs_data_get_obj(eld, "value");
				if (vector) {
					param.intArray[0] = (int32_t)obs_data_get_int(vector, "x");
					param.intArray[1] = (int32_t)obs_data_get_int(vector, "y");
					obs_data_release(vector);
				}
				m_parameters.emplace(parameterName, param);
			} else if (type == "integer3") {
				Parameter param;
				param.type = GS::EffectParameter::Type::Integer3;
				obs_data_t* vector = obs_data_get_obj(eld, "value");
				if (vector) {
					param.intArray[0] = (int32_t)obs_data_get_int(vector, "x");
					param.intArray[1] = (int32_t)obs_data_get_int(vector, "y");
					param.intArray[2] = (int32_t)obs_data_get_int(vector, "z");
					obs_data_release(vector);
				}
				m_parameters.emplace(parameterName, param);
			} else if (type == "integer4") {
				Parameter param;
				param.type = GS::EffectParameter::Type::Integer4;
				obs_data_t* vector = obs_data_get_obj(eld, "value");
				if (vector) {
					param.intArray[0] = (int32_t)obs_data_get_int(vector, "x");
					param.intArray[1] = (int32_t)obs_data_get_int(vector, "y");
					param.intArray[2] = (int32_t)obs_data_get_int(vector, "z");
					param.intArray[3] = (int32_t)obs_data_get_int(vector, "w");
					obs_data_release(vector);
				}
				m_parameters.emplace(parameterName, param);
			} else if (type == "integer[]") {
				// ToDo
			} else if (type == "float") {
				Parameter param;
				param.type = GS::EffectParameter::Type::Float;
				param.floatValue = (float_t)obs_data_get_double(eld, "value");
				m_parameters.emplace(parameterName, param);
			} else if (type == "float2") {
				Parameter param;
				param.type = GS::EffectParameter::Type::Float2;
				obs_data_t* vector = obs_data_get_obj(eld, "value");
				if (vector) {
					param.floatArray[0] = (float_t)obs_data_get_double(vector, "x");
					param.floatArray[1] = (float_t)obs_data_get_double(vector, "y");
					obs_data_release(vector);
				}
				m_parameters.emplace(parameterName, param);
			} else if (type == "float3") {
				Parameter param;
				param.type = GS::EffectParameter::Type::Float3;
				obs_data_t* vector = obs_data_get_obj(eld, "value");
				if (vector) {
					param.floatArray[0] = (float_t)obs_data_get_double(vector, "x");
					param.floatArray[1] = (float_t)obs_data_get_double(vector, "y");
					param.floatArray[2] = (float_t)obs_data_get_double(vector, "z");
					obs_data_release(vector);
				}
				m_parameters.emplace(parameterName, param);
			} else if (type == "float4") {
				Parameter param;
				param.type = GS::EffectParameter::Type::Float4;
				obs_data_t* vector = obs_data_get_obj(eld, "value");
				if (vector) {
					param.floatArray[0] = (float_t)obs_data_get_double(vector, "x");
					param.floatArray[1] = (float_t)obs_data_get_double(vector, "y");
					param.floatArray[2] = (float_t)obs_data_get_double(vector, "z");
					param.floatArray[3] = (float_t)obs_data_get_double(vector, "w");
					obs_data_release(vector);
				}
				m_parameters.emplace(parameterName, param);
			} else if (type == "float[]") {
				// ToDo
			} else if (type == "matrix") {
				Parameter param;
				param.type = GS::EffectParameter::Type::Matrix;
				obs_data_t* vector = obs_data_get_obj(eld, "value");
				if (vector) {
					obs_data_get_vec4(vector, "x", &(param.matrix.x));
					obs_data_get_vec4(vector, "y", &(param.matrix.x));
					obs_data_get_vec4(vector, "z", &(param.matrix.x));
					obs_data_get_vec4(vector, "w", &(param.matrix.x));
					obs_data_release(vector);
				}
				m_parameters.emplace(parameterName, param);
			}
			obs_data_release(eld);
		}
		if(prmd)
			obs_data_release(prmd);
	}
}

Mask::Resource::Material::~Material() {
	if (m_samplerState)
		gs_samplerstate_destroy(m_samplerState);
}

Mask::Resource::Type Mask::Resource::Material::GetType() {
	return Mask::Resource::Type::Material;
}

void Mask::Resource::Material::Update(Mask::Part* part, float time) {
	// update our image params
	m_parent->instanceDatas.Push(m_id);
	for (auto kv : m_imageParameters) {
		kv.second->Update(part, time);
	}
	m_parent->instanceDatas.Pop();
}

void Mask::Resource::Material::Render(Mask::Part* part) {
	UNUSED_PARAMETER(part);
	return;
}

bool Mask::Resource::Material::Loop(Mask::Part* part, BonesList* bones) {

	m_parent->instanceDatas.Push(m_id);
	if (!m_looping) {

		m_effect->Render(part);

		// Apply Parameters
		for (auto kv : m_parameters) {
			try {
				auto el = m_effect->GetEffect()->GetParameterByName(kv.first);
				switch (kv.second.type) {
				case GS::EffectParameter::Type::Float:
					el.SetFloat(kv.second.floatValue);
					break;
				case GS::EffectParameter::Type::Float2:
					el.SetFloat2(kv.second.floatArray[0],
						kv.second.floatArray[1]);
					break;
				case GS::EffectParameter::Type::Float3:
					el.SetFloat3(kv.second.floatArray[0],
						kv.second.floatArray[1],
						kv.second.floatArray[2]);
					break;
				case GS::EffectParameter::Type::Float4:
					el.SetFloat4(kv.second.floatArray[0],
						kv.second.floatArray[1], 
						kv.second.floatArray[2], 
						kv.second.floatArray[3]);
					break;
				case GS::EffectParameter::Type::Integer:
					el.SetInteger(kv.second.intValue);
					break;
				case GS::EffectParameter::Type::Integer2:
					el.SetInteger2(kv.second.intArray[0],
						kv.second.intArray[1]);
					break;
				case GS::EffectParameter::Type::Integer3:
					el.SetInteger3(kv.second.intArray[0],
						kv.second.intArray[1], 
						kv.second.intArray[2]);
					break;
				case GS::EffectParameter::Type::Integer4:
					el.SetInteger4(kv.second.intArray[0],
						kv.second.intArray[1],
						kv.second.intArray[2],
						kv.second.intArray[3]);
					break;
				case GS::EffectParameter::Type::Matrix:
					el.SetMatrix(kv.second.matrix);
					break;
				}
			}
			catch (...) {
			}
		}

		// get the effect object
		gs_effect_t* eff = m_effect->GetEffect()->GetObject();

		// Set up the sampler state
		// TODO: move this to gs::effectparameter?
		if (!m_samplerState) {
			gs_sampler_info sinfo;
			sinfo.address_u = m_wrapU;
			sinfo.address_v = m_wrapV;
			sinfo.address_w = m_wrapW;
			sinfo.filter = m_filter;
			sinfo.border_color = 0;
			sinfo.max_anisotropy = 4;
			m_samplerState = gs_samplerstate_create(&sinfo);
		}

		// Set material rendering flags
		gs_enable_depth_test(m_depthTest != gs_depth_test::GS_ALWAYS);
		gs_depth_function(m_depthTest);
		gs_set_cull_mode(m_culling);
		
		// set up image params (image/sequence)
		bool texmatset = false;
		for (auto kv : m_imageParameters) {
			try {
				auto el = m_effect->GetEffect()->GetParameterByName(kv.first);
				if (kv.second->GetType() == Type::Image) {
					std::shared_ptr<Image> img = std::dynamic_pointer_cast<Image>(kv.second);
					img->Render(part);
					el.SetTexture(img->GetTexture());
					el.SetSampler(m_samplerState);
				}
				else if (kv.second->GetType() == Type::Sequence) {
					std::shared_ptr<Sequence> seq = std::dynamic_pointer_cast<Sequence>(kv.second);
					seq->Render(part);
					std::shared_ptr<Image> img = seq->GetImage();
					el.SetTexture(img->GetTexture());
					el.SetSampler(m_samplerState);

					// set up texture matrix
					// NOTE: there is no gs_effect_set_matrix3. bummer.
					// - might be better as translate/scale/rot (vec2/vec2/float)
					matrix4 texmat;
					seq->SetTextureMatrix(part, &texmat);
					gs_eparam_t* effparm = gs_effect_get_param_by_name(eff, PARAM_TEXMAT);
					texmatset = true;
					if (effparm)
						gs_effect_set_matrix4(effparm, &texmat);
				}
			}
			catch (...) {
			}
		}

		// set params for lighting
		SetLightingParameters(part);

		// set params for skinning
		SetSkinningParameters(bones);

		// set global alpha
		gs_eparam_t* effparm = gs_effect_get_param_by_name(eff, PARAM_ALPHA);
		if (effparm) {
			std::shared_ptr<AlphaInstanceData> aid =
				m_parent->instanceDatas.GetData<AlphaInstanceData>
				(AlphaInstanceDataId);
			gs_effect_set_float(effparm, aid->alpha);
		}

		// get the technique
		m_currentTechnique = gs_effect_get_technique(m_effect->GetEffect()->GetObject(),
			m_technique.c_str());
		if (!m_currentTechnique) {
			m_parent->instanceDatas.Pop();
			return false;
		}

		// begin
		m_techniquePasses = gs_technique_begin(m_currentTechnique);
		m_techniquePass = 0;
		m_looping = true;
	} else {
		gs_technique_end_pass(m_currentTechnique);
	}

	if (!gs_technique_begin_pass(m_currentTechnique, m_techniquePass++)
		|| (m_techniquePass > m_techniquePasses)) {
		gs_technique_end(m_currentTechnique);
		m_looping = false;
		m_parent->instanceDatas.Pop();
		return false;
	}

	m_parent->instanceDatas.Pop();
	return true;
}

gs_address_mode Mask::Resource::Material::StringToAddressMode(std::string s) {
	if (s == "clamp")
		return GS_ADDRESS_CLAMP;
	else if (s == "wrap")
		return GS_ADDRESS_WRAP;
	else if (s == "mirror")
		return GS_ADDRESS_MIRROR;
	else if (s == "border")
		return GS_ADDRESS_BORDER;
	else if (s == "mirroronce")
		return GS_ADDRESS_MIRRORONCE;

	return GS_ADDRESS_CLAMP; 
}

void Mask::Resource::Material::SetLightingParameters(Mask::Part* part) {
	UNUSED_PARAMETER(part);
	// get the effect object
	gs_effect_t* eff = m_effect->GetEffect()->GetObject();
	
	gs_eparam_t* param;

	// Set up world matrix for lighting 
	param = gs_effect_get_param_by_name(eff, PARAM_WORLD);
	if (param) {
		// go obs. need to transpose matrices sent to shaders from gs.
		matrix4 w;
		gs_matrix_get(&w);
		matrix4_transpose(&w, &w);
		gs_effect_set_matrix4(param, &w);
	}
	else {
		// No World matrix param - assume this effect doesn't support
		// lighting
		return;
	}


	// Look for light instance data
	char temp[64];
	int numLights = 0;
	for (int i = 0; i < 8; i++) {

		std::shared_ptr<LightInstanceData> lightData =
			m_parent->instanceDatas.FindDataDontCreate<LightInstanceData>(m_lightIds[i]);
		if (!lightData)
			break;

		numLights++;

		// light type
		snprintf(temp, sizeof(temp), "light%dType", i);
		param = gs_effect_get_param_by_name(eff, temp);
		if (param)
			gs_effect_set_int(param, (int)lightData->lightType);
		
		// position
		snprintf(temp, sizeof(temp), "light%dPosition", i);
		param = gs_effect_get_param_by_name(eff, temp);
		if (param)
			gs_effect_set_vec3(param, &lightData->position);

		// direction
		snprintf(temp, sizeof(temp), "light%dDirection", i);
		param = gs_effect_get_param_by_name(eff, temp);
		if (param)
			gs_effect_set_vec3(param, &lightData->direction);

		// attenuation
		snprintf(temp, sizeof(temp), "light%dAttenuation", i);
		param = gs_effect_get_param_by_name(eff, temp);
		if (param) { 
			vec3 att;
			vec3_set(&att, lightData->att0, lightData->att1, lightData->att2);
			gs_effect_set_vec3(param, &att);
		}
		 
		// ambient color
		snprintf(temp, sizeof(temp), "light%dAmbient", i);
		param = gs_effect_get_param_by_name(eff, temp);
		if (param) 
			gs_effect_set_vec3(param, &lightData->ambient);
		
		// diffuse color
		snprintf(temp, sizeof(temp), "light%dDiffuse", i);
		param = gs_effect_get_param_by_name(eff, temp);
		if (param)
			gs_effect_set_vec3(param, &lightData->diffuse);
		
		// specular color 
		snprintf(temp, sizeof(temp), "light%dSpecular", i);
		param = gs_effect_get_param_by_name(eff, temp);
		if (param)
			gs_effect_set_vec3(param, &lightData->specular); 
		 
		// spot angle
		snprintf(temp, sizeof(temp), "light%dAngle", i);
		param = gs_effect_get_param_by_name(eff, temp);
		if (param)
			gs_effect_set_float(param, lightData->outerAngle / 2.0f);
	} 

	// num lights
	if (numLights > 0) {
		param = gs_effect_get_param_by_name(eff, PARAM_NUMLIGHTS);
		if (param)
			gs_effect_set_int(param, numLights);
	}
}


void Mask::Resource::Material::SetSkinningParameters(
	Mask::Resource::BonesList* bones) {
	// get the effect object
	gs_effect_t* eff = m_effect->GetEffect()->GetObject();
	gs_eparam_t* param;

	if (bones == nullptr || bones->numBones < 1) {
		// numBones
		param = gs_effect_get_param_by_name(eff, PARAM_NUMBONES);
		if (param)
			gs_effect_set_int(param, 0);
		return;
	}

	char temp[64];
	int nb = bones->numBones;

	// numBones
	param = gs_effect_get_param_by_name(eff, PARAM_NUMBONES);
	if (param)
		gs_effect_set_int(param, nb);

	// bone matrices
	for (int i = 0; i < nb; i++) {
		snprintf(temp, sizeof(temp), "bone%d", i);
		param = gs_effect_get_param_by_name(eff, temp);
		if (param) {
			gs_effect_set_matrix4(param, bones->bones[i]);
		}
	}
}
