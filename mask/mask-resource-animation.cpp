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

static const char* const S_DURATION = "duration";
static const char* const S_FPS = "fps";
static const char* const S_CHANNELS = "channels";
static const char* const S_NAME = "name";
static const char* const S_TYPE = "type";
static const char* const S_PRESTATE = "pre-state";
static const char* const S_POSTSTATE = "post-state";
static const char* const S_VALUES = "values";


float	Mask::Resource::AnimationChannel::GetValue(int frame) {
	float v = 0.0f;

	if (frame < 0) {
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
			while (frame < 0)
				frame += (int)values.size();
			if (frame < values.size())
				v = values[frame];
			break;
		}
	}
	else if (frame >= values.size()) {
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
				frame = frame % values.size();
				v = values[frame];
			}
			break;
		}
	}
	else {
		v = values[frame];
	}

	return v;
}




Mask::Resource::Animation::Animation(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name), m_speed(1.0f), m_stopOnLastFrame(false) {

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

	// WOOPS - I wrongly assumed the duration would be in seconds. It is in frames.
	// - By the time I realized this, too many masks were already in circulation.
	// - Just correct it here
	m_duration /= m_fps;

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
			obs_data_release(chand);
			obs_data_release(channels);
			PLOG_ERROR("Animation '%s' channel has no name.", name.c_str());
			throw std::logic_error("Animation channel has no name.");
		}
		std::string itemName = obs_data_get_string(chand, S_NAME);

		if (!obs_data_has_user_value(chand, S_TYPE)) {
			obs_data_release(chand);
			obs_data_release(channels);
			PLOG_ERROR("Animation '%s' channel has no type.", name.c_str());
			throw std::logic_error("Animation channel has no type.");
		}
		channel.type = AnimationTypeFromString(obs_data_get_string(chand, S_TYPE));

		// Set item based on channel type
		if (channel.type < PART_CHANNEL_LAST)
			// Item is a part
			channel.item = parent->GetPart(itemName);
		else {
			// Item is a morph
			std::shared_ptr<Resource::Morph> morph = 
				std::dynamic_pointer_cast<Resource::Morph>(parent->GetResource(itemName));
			channel.item = morph;
		}

		if (!obs_data_has_user_value(chand, S_PRESTATE)) {
			obs_data_release(chand);
			obs_data_release(channels);
			PLOG_ERROR("Animation '%s' channel has no pre state.", name.c_str());
			throw std::logic_error("Animation channel has no pre state.");
		}
		channel.preState = AnimationBehaviourFromString(obs_data_get_string(chand, S_PRESTATE));

		if (!obs_data_has_user_value(chand, S_POSTSTATE)) {
			obs_data_release(chand);
			obs_data_release(channels);
			PLOG_ERROR("Animation '%s' channel has no pose state.", name.c_str());
			throw std::logic_error("Animation channel has no post state.");
		}
		channel.postState = AnimationBehaviourFromString(obs_data_get_string(chand, S_POSTSTATE));

		if (!obs_data_has_user_value(chand, S_VALUES)) {
			obs_data_release(chand);
			obs_data_release(channels);
			PLOG_ERROR("Animation '%s' channel has no values.", name.c_str());
			throw std::logic_error("Animation channel has no values.");
		}
		const char* base64data = obs_data_get_string(chand, S_VALUES);
		if (base64data[0] == '\0') {
			obs_data_release(chand);
			obs_data_release(channels);
			PLOG_ERROR("Animation '%s' channel has empty values data.", name.c_str());
			throw std::logic_error("Animation channel has empty values data.");
		}
		std::vector<uint8_t> decoded;
		base64_decodeZ(base64data, decoded);
		size_t numFloats = decoded.size() / sizeof(float);
		channel.values.assign((float*)decoded.data(), (float*)decoded.data() + numFloats);

		m_channels.emplace_back(channel);
		obs_data_release(chand);
	}
	obs_data_release(channels);
}

Mask::Resource::Animation::~Animation() {}

Mask::Resource::Type Mask::Resource::Animation::GetType() {
	return Mask::Resource::Type::Animation;
}

void Mask::Resource::Animation::Update(Mask::Part* part, float time) {
	UNUSED_PARAMETER(part);
	m_parent->instanceDatas.Push(m_id);

	// get our instance data
	std::shared_ptr<AnimationInstanceData> instData =
		m_parent->instanceDatas.GetData<AnimationInstanceData>();

	// time has elapsed
	instData->elapsed += time * m_speed;

	// stop on last frame?
	if (m_stopOnLastFrame) {
		if (m_speed > 0.0f) {
			if (instData->elapsed >= LastFrame()) {
				instData->elapsed = LastFrame();
				Stop();
			}
		}
		else {
			if (instData->elapsed <= 0) {
				instData->elapsed = 0;
				Stop();
			}
		}
	}

	// process animation channels
	int frame = (int)(instData->elapsed * m_fps);
	for (int i = 0; i < m_channels.size(); i++) {
		AnimationChannel& ch = m_channels[i];
		if (ch.item != nullptr) {
			ch.item->SetAnimatableValue(ch.GetValue(frame), ch.type);
		}
	}

	m_parent->instanceDatas.Pop();
}

void Mask::Resource::Animation::Render(Mask::Part* part) {
	UNUSED_PARAMETER(part);
	return;
}

void Mask::Resource::Animation::Seek(float t) {
	m_parent->instanceDatas.Push(m_id);

	// get our instance data
	std::shared_ptr<AnimationInstanceData> instData =
		m_parent->instanceDatas.GetData<AnimationInstanceData>();

	// reset time
	instData->elapsed = t;

	m_parent->instanceDatas.Pop();
}

float   Mask::Resource::Animation::GetPosition() {
	m_parent->instanceDatas.Push(m_id);

	// get our instance data
	std::shared_ptr<AnimationInstanceData> instData =
		m_parent->instanceDatas.GetData<AnimationInstanceData>();

	m_parent->instanceDatas.Pop();

	return instData->elapsed;
}


Mask::Resource::AnimationChannelType 
Mask::Resource::Animation::AnimationTypeFromString(const std::string& s) {

	// Part channels
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

	// Morph channels (of form morph-32-x morph-33-y etc)
	if (s.substr(0, 5) == "morph") {
		int id = 0;
		std::vector<std::string> bits = Utils::split(s, '-');
		if (bits.size() == 3) {
			int idx = atoi(bits[1].c_str());
			if (bits[2] == "x")
				id = MORPH_LANDMARK_0_X + (3 * idx);
			else if (bits[2] == "y")
				id = MORPH_LANDMARK_0_Y + (3 * idx);
			else 
				id = MORPH_LANDMARK_0_Z + (3 * idx);
			return (Mask::Resource::AnimationChannelType)id;
		}
	}

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


