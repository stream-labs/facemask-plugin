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

#include "mask-resource-light.h"
#include "exceptions.h"
#include "plugin.h"
#include "utils.h"


Mask::Resource::LightInstanceData& 
Mask::Resource::LightInstanceData::operator=(const LightInstanceData& other) {
	lightType = other.lightType;
	position = other.position;
	direction = other.direction;
	up = other.up;
	att0 = other.att0;
	att1 = other.att1;
	att2 = other.att2;
	ambient = other.ambient;
	diffuse = other.diffuse;
	specular = other.specular;
	innerAngle = other.innerAngle;
	outerAngle = other.outerAngle;
	areaSize = other.areaSize;
	return *this;
}


Mask::Resource::Light::Light(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name) {
	
	// Get the light type
	if (!obs_data_has_user_value(data, S_LIGHTTYPE)) {
		PLOG_ERROR("Light '%s' has no light-type value.", name.c_str());
		throw std::logic_error("Light has no light-type value.");
	}
	m_idat.lightType = GetLightType(data);

	// Position/Attenuation (not valid for ambient/directional lights)
	if (m_idat.lightType != DIRECTIONAL && m_idat.lightType != AMBIENT) {
		// position
		if (!obs_data_has_user_value(data, S_POSITION)) {
			PLOG_ERROR("Light '%s' has no position value.", name.c_str());
			throw std::logic_error("Light has no position value.");
		}
		obs_data_get_vec3(data, S_POSITION, &m_idat.position);

		// attenuation values
		if (!obs_data_has_user_value(data, S_ATT0) ||
			!obs_data_has_user_value(data, S_ATT1) ||
			!obs_data_has_user_value(data, S_ATT2)) {
			PLOG_ERROR("Light '%s' is missing attenuation values.", name.c_str());
			throw std::logic_error("Light is missing attenuation values.");
		}
		m_idat.att0 = (float)obs_data_get_double(data, S_ATT0);
		m_idat.att1 = (float)obs_data_get_double(data, S_ATT1);
		m_idat.att2 = (float)obs_data_get_double(data, S_ATT2);
	}

	// Direction/Up Vectors (not valid for ambient/point lights)
	if (m_idat.lightType != POINT && m_idat.lightType != AMBIENT) {
		// direction
		if (!obs_data_has_user_value(data, S_DIRECTION)) {
			PLOG_ERROR("Light '%s' has no direction value.", name.c_str());
			throw std::logic_error("Light has no direction value.");
		}
		obs_data_get_vec3(data, S_DIRECTION, &m_idat.direction);

		// up
		if (!obs_data_has_user_value(data, S_UP)) {
			PLOG_ERROR("Light '%s' has no up value.", name.c_str());
			throw std::logic_error("Light has no up value.");
		}
		obs_data_get_vec3(data, S_UP, &m_idat.up);
	}

	// Ambient Color
	if (!obs_data_has_user_value(data, S_AMBIENT)) {
		PLOG_ERROR("Light '%s' has no ambient color value.", name.c_str());
		throw std::logic_error("Light has no ambient color value.");
	}
	obs_data_get_vec3(data, S_AMBIENT, &m_idat.ambient);

	// Diffuse Color
	if (!obs_data_has_user_value(data, S_DIFFUSE)) {
		PLOG_ERROR("Light '%s' has no diffuse color value.", name.c_str());
		throw std::logic_error("Light has no diffuse color value.");
	}
	obs_data_get_vec3(data, S_DIFFUSE, &m_idat.diffuse);

	// Specular Color
	if (!obs_data_has_user_value(data, S_SPECULAR)) {
		PLOG_ERROR("Light '%s' has no specular color value.", name.c_str());
		throw std::logic_error("Light has no specular color value.");
	}
	obs_data_get_vec3(data, S_SPECULAR, &m_idat.specular);

	// Spot Angles (only valid for spot lights)
	if (m_idat.lightType == SPOT) {
		if (!obs_data_has_user_value(data, S_INNERANGLE) ||
			!obs_data_has_user_value(data, S_OUTERANGLE)) {
			PLOG_ERROR("Spot Light '%s' is missing angle value(s).", name.c_str());
			throw std::logic_error("Light is missing angle value(s).");
		}

		m_idat.innerAngle = (float)obs_data_get_double(data, S_INNERANGLE);
		m_idat.outerAngle = (float)obs_data_get_double(data, S_OUTERANGLE);
	}

	// Area Size (only valid for area lights)
	if (m_idat.lightType == AREA) {
		if (!obs_data_has_user_value(data, S_AREASIZE)) {
			PLOG_ERROR("Area Light '%s' has no size value.", name.c_str());
			throw std::logic_error("Light has no size value.");
		}
		obs_data_get_vec2(data, S_AREASIZE, &m_idat.areaSize);
	} 
}

Mask::Resource::Light::~Light() {}

Mask::Resource::Type Mask::Resource::Light::GetType() {
	return Mask::Resource::Type::Light;
} 

void Mask::Resource::Light::Update(Mask::Part* part, float time) {
	UNUSED_PARAMETER(time);

	// get our instance data
	// NOTE: lights are global; not instanced
	std::shared_ptr<LightInstanceData> instData =
		m_parent->instanceDatas.GetData<LightInstanceData>(m_id);
	if (instData->lightType == UNDEFINED) {
		// initialize light data
		*instData = m_idat;
	}

	// transform position and direction
	matrix4 g; 
	matrix4_copy(&g, &part->global);

	vec3_transform(&instData->position, &m_idat.position, &g);
	vec4 d, t;
	vec4_from_vec3(&d, &m_idat.direction);
	d.w = 0.0f;
	vec4_transform(&t, &d, &g);
	vec3_from_vec4(&instData->direction, &t); 
}

void Mask::Resource::Light::Render(Mask::Part* part) {
	UNUSED_PARAMETER(part);
	return;
}

Mask::Resource::LightType Mask::Resource::Light::GetLightType(obs_data_t* data) {
	std::string t = obs_data_get_string(data, S_LIGHTTYPE);
	if (t == "ambient")
		return AMBIENT;
	else if (t == "directional")
		return DIRECTIONAL;
	else if (t == "point")
		return POINT;
	else if (t == "spot")
		return SPOT;
	else if (t == "area") {
		PLOG_ERROR("Area lights are not supported.");
		throw std::logic_error("Area lights are not supported.");
		return AREA;
	}
	PLOG_ERROR("Light type '%s' is invalid.", t.c_str());
	throw std::logic_error("Light type is invalid.");

	return AMBIENT;
}
