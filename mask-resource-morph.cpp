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

#include "mask-resource-morph.h"
#include "mask.h"
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

static const char* const S_DELTAS = "deltas";


Mask::Resource::Morph::Morph(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name) {
	
	// Deltas list
	if (!obs_data_has_user_value(data, S_DELTAS)) {
		PLOG_ERROR("Morph '%s' has no deltas list.", name.c_str());
		throw std::logic_error("Morph has no deltas.");
	}
	obs_data_item_t* deltasItem = obs_data_item_byname(data, S_DELTAS);
	if (obs_data_item_gettype(deltasItem) != obs_data_type::OBS_DATA_OBJECT) {
		PLOG_ERROR("Bad deltas section in '%s'.", name.c_str());
		throw std::logic_error("Morph has bad deltas section.");
	}
	obs_data_t* deltasData = obs_data_item_get_obj(deltasItem);
	if (!deltasData) {
		PLOG_ERROR("Bad deltas section in '%s'.", name.c_str());
		throw std::logic_error("Morph has bad deltas section.");
	}
	 
	int numPoints = 0;
	smll::DeltaList& deltas = m_morphData.GetDeltasAndStamp();
	for (obs_data_item_t* itm = obs_data_first(deltasData); itm; obs_data_item_next(&itm)) {
		// use string key as index into array
		std::string nn = obs_data_item_get_name(itm);
		int idx = atoi(nn.c_str());

		// position
		vec3 position;
		obs_data_get_vec3(deltasData, nn.c_str(), &position);
		deltas[idx] = position;

		// sanity check
		numPoints++;
	}
	// sanity check
	if (numPoints > smll::NUM_FACIAL_LANDMARKS) {
		PLOG_ERROR("Bad deltas section in '%s'. Too many deltas.", name.c_str());
		throw std::logic_error("Morph has bad deltas section. Too many deltas.");
	}
}

Mask::Resource::Morph::~Morph() {}

Mask::Resource::Type Mask::Resource::Morph::GetType() {
	return Mask::Resource::Type::Morph;
}

void Mask::Resource::Morph::Update(Mask::Part* part, float time) {
	UNUSED_PARAMETER(part);
	UNUSED_PARAMETER(time);
}

void Mask::Resource::Morph::Render(Mask::Part* part) {
	UNUSED_PARAMETER(part);

	// TODO: move rendering in here
}

bool Mask::Resource::Morph::IsDepthOnly() {
	return false;
}

void Mask::Resource::Morph::SetAnimatableValue(float v,
	Mask::Resource::AnimationChannelType act) {
	UNUSED_PARAMETER(act);
	UNUSED_PARAMETER(v);
	// TODO: SET VALUES
}