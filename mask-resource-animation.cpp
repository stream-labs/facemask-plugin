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
#include "mask-resource-animation.h"
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

static const char* const S_DURATION = "duration";
static const char* const S_FPS = "fps";
static const char* const S_CHANNELS = "channels";
static const char* const S_NAME = "name";
static const char* const S_TYPE = "type";
static const char* const S_PRESTATE = "pre-state";
static const char* const S_POSTSTATE = "post-state";
static const char* const S_VALUES = "values";


float	Mask::Resource::AnimationChannel::GetValue(float time) {
	float v = 0.0f;
	size_t i = (size_t)time;

	if (i < 0) {
		switch (preState) {
		case CONSTANT:
			if (values.size() > 0)
				v = values[0];
			break;
		case LINEAR:
			// todo
			if (values.size() > 0)
				v = values[0];
			break;
		case REPEAT:
			while (i < 0)
				i += values.size();
			if (i < values.size())
				v = values[i];
			break;
		}
	}
	else if (i >= values.size()) {
		switch (postState) {
		case CONSTANT:
			if (values.size() > 0)
				v = values[values.size() - 1];
			break;
		case LINEAR:
			// TODO
			if (values.size() > 0)
				v = values[values.size() - 1];
			break;
		case REPEAT:
			if (values.size() > 0) {
				i = i % values.size();
				v = values[i];
			}
			break;
		}
	}
	else {
		v = values[i];
	}

	return v;
}




Mask::Resource::Animation::Animation(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name) {

	if (!obs_data_has_user_value(data, S_DURATION)) {
		PLOG_ERROR("Animation '%s' has no duration.", name.c_str());
		throw std::logic_error("Animation has no duration.");
	}
	m_duration = (float)obs_data_get_double(data, S_DURATION);

	if (!obs_data_has_user_value(data, S_FPS)) {
		PLOG_ERROR("Animation '%s' has no fps.", name.c_str());
		throw std::logic_error("Animation has no fps.");
	}
	m_fps = (float)obs_data_get_double(data, S_FPS);

	if (!obs_data_has_user_value(data, S_CHANNELS)) {
		PLOG_ERROR("Animation '%s' has no channels.", name.c_str());
		throw std::logic_error("Animation has no channels.");
	}
	obs_data_t* channels = obs_data_get_obj(data, S_CHANNELS);
	for (obs_data_item_t* el = obs_data_first(channels); el; obs_data_item_next(&el)) {
		std::string channelName = obs_data_item_get_name(el);
		obs_data_t* chand = obs_data_item_get_obj(el);
		if (!chand)
			continue;
		 
		AnimationChannel channel;

		if (!obs_data_has_user_value(chand, S_NAME)) {
			PLOG_ERROR("Animation '%s' channel has no name.", name.c_str());
			throw std::logic_error("Animation channel has no name.");
		}
		channel.name = obs_data_get_string(chand, S_NAME);

		if (!obs_data_has_user_value(chand, S_TYPE)) {
			PLOG_ERROR("Animation '%s' channel has no type.", name.c_str());
			throw std::logic_error("Animation channel has no type.");
		}
		channel.type = AnimationTypeFromString(obs_data_get_string(chand, S_TYPE));

		if (!obs_data_has_user_value(chand, S_PRESTATE)) {
			PLOG_ERROR("Animation '%s' channel has no pre state.", name.c_str());
			throw std::logic_error("Animation channel has no pre state.");
		}
		channel.preState = AnimationBehaviourFromString(obs_data_get_string(chand, S_PRESTATE));

		if (!obs_data_has_user_value(chand, S_POSTSTATE)) {
			PLOG_ERROR("Animation '%s' channel has no pose state.", name.c_str());
			throw std::logic_error("Animation channel has no post state.");
		}
		channel.postState = AnimationBehaviourFromString(obs_data_get_string(chand, S_POSTSTATE));

		if (!obs_data_has_user_value(chand, S_VALUES)) {
			PLOG_ERROR("Animation '%s' channel has no values.", name.c_str());
			throw std::logic_error("Animation channel has no values.");
		}
		const char* base64data = obs_data_get_string(chand, S_VALUES);
		if (base64data[0] == '\0') {
			PLOG_ERROR("Animation '%s' channel has empty values data.", name.c_str());
			throw std::logic_error("Animation channel has empty values data.");
		}
		std::vector<uint8_t> decoded = base64_decodeZ(base64data);
		size_t numFloats = decoded.size() / sizeof(float);
		channel.values.assign((float*)decoded.data(), (float*)decoded.data() + numFloats);

		m_channels.emplace_back(channel);
	}
}

Mask::Resource::Animation::~Animation() {}

Mask::Resource::Type Mask::Resource::Animation::GetType() {
	return Mask::Resource::Type::Animation;
}

void Mask::Resource::Animation::Update(Mask::Part* part, float time) {

	part->mask->instanceDatas.Push(m_id);

	// get our instance data
	std::shared_ptr<AnimationInstanceData> instData =
		part->mask->instanceDatas.GetData<AnimationInstanceData>();

	// time has elapsed
	instData->elapsed += time;

	// process animation channels
	float t = instData->elapsed * m_fps;
	for (int i = 0; i < m_channels.size(); i++) {
		AnimationChannel& ch = m_channels[i];

		// all channels are parts ATM...this may change
		std::shared_ptr<Mask::Part> ppp = part->mask->GetPart(ch.name);
		if (ppp) {
			// set value
			float v = ch.GetValue(t);
			switch (ch.type) {
			case PART_POSITION_X:
				ppp->position.x = v;
				break;
			case PART_POSITION_Y:
				ppp->position.y = v;
				break;
			case PART_POSITION_Z:
				ppp->position.z = v;
				break;
			case PART_QROTATION_X:
				ppp->qrotation.x = v;
				break;
			case PART_QROTATION_Y:
				ppp->qrotation.y = v;
				break;
			case PART_QROTATION_Z:
				ppp->qrotation.z = v;
				break;
			case PART_QROTATION_W:
				ppp->qrotation.w = v;
				break;
			case PART_SCALE_X:
				ppp->scale.x = v;
				break;
			case PART_SCALE_Y:
				ppp->scale.y = v;
				break;
			case PART_SCALE_Z:
				ppp->scale.z = v;
				break;
			}
			ppp->localdirty = true;
		}
	}

	part->mask->instanceDatas.Pop();
}

void Mask::Resource::Animation::Render(Mask::Part* part) {
	UNUSED_PARAMETER(part);
	return;
}

Mask::Resource::AnimationChannelType 
Mask::Resource::Animation::AnimationTypeFromString(const std::string& s) {
	if (s == "part-pos-x")
		return PART_POSITION_X;
	if (s == "part-pos-y")
		return PART_POSITION_Y;
	if (s == "part-pos-z")
		return PART_POSITION_Z;
	if (s == "part-qrot-x")
		return PART_QROTATION_X;
	if (s == "part-qrot-y")
		return PART_QROTATION_Y;
	if (s == "part-qrot-z")
		return PART_QROTATION_Z;
	if (s == "part-qrot-w")
		return PART_QROTATION_W;
	if (s == "part-scl-x")
		return PART_SCALE_X;
	if (s == "part-scl-y")
		return PART_SCALE_Y;
	if (s == "part-scl-z")
		return PART_SCALE_Z;

	return INVALID_TYPE;
}

Mask::Resource::AnimationBehaviour
Mask::Resource::Animation::AnimationBehaviourFromString(const std::string& s) {
	if (s == "constant")
		return CONSTANT;
	if (s == "linear")
		return LINEAR;
	if (s == "repeat")
		return REPEAT;
	return CONSTANT;
}
