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
#define NOMINMAX

#include "mask.h"
#include "mask-resource-animation.h"
#include "mask-resource-animation-target.h"
#include "plugin/exceptions.h"
#include "plugin/plugin.h"
#include "plugin/utils.h"

Mask::Resource::AnimationTarget::AnimationTarget(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name) {

	if (!obs_data_has_user_value(data, S_DURATION)) {
		PLOG_ERROR("Animation Target '%s' has no duration.", name.c_str());
		throw std::logic_error("Animation has no duration.");
	}
	m_frames_count = (float)obs_data_get_double(data, S_DURATION);

	if (!obs_data_has_user_value(data, S_FPT)) {
		PLOG_ERROR("Animation Target '%s' has no fps.", name.c_str());
		throw std::logic_error("Animation has no fps.");
	}
	m_frames_per_target = (float)obs_data_get_double(data, S_FPT);

	if (!obs_data_has_user_value(data, S_CHANNELS)) {
		PLOG_ERROR("Animation Target '%s' has no channels.", name.c_str());
		throw std::logic_error("Animation Target has no channels.");
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

	// clean out target channels
	int target_count = std::ceil(m_frames_count / m_frames_per_target);
	bool target_includes_channel;
	
	for (size_t i = 0; i < m_channels.size(); i++)
	{
		std::unordered_set<int> current_targets;
		for (size_t k = 0; k < target_count; k++)
		{
			target_includes_channel = false;
			for (size_t f = k*m_frames_per_target + 1; f < (k+1) * m_frames_per_target; f++)
			{
				if (!(std::abs(m_channels[i].values[f] - m_channels[i].values[f-1]) <=
					  std::max(std::abs(m_channels[i].values[f]), std::abs(m_channels[i].values[f-1]))* std::numeric_limits<float>::epsilon()
				   ))
				{
					// nequal
					target_includes_channel = true;
					break;
				}
			}
			if (target_includes_channel)
				current_targets.insert(k);
		}
		channel_targets.emplace_back(current_targets);
	}

	for (size_t i = 0; i < m_channels.size(); i++)
	{
		if (m_channels[i].type >= AnimationChannelType::PART_QROTATION_X &&
			m_channels[i].type <= AnimationChannelType::PART_QROTATION_W)
		{
			if(quaternion_channel_indices_map.find(m_channels[i].item) == quaternion_channel_indices_map.end())
				quaternion_channel_indices_map[m_channels[i].item] = std::array<size_t, 4>();

			quaternion_channel_indices_map[m_channels[i].item][m_channels[i].type - PART_QROTATION_X] = i;
		}
	}
}

Mask::Resource::AnimationTarget::~AnimationTarget() {}

Mask::Resource::Type Mask::Resource::AnimationTarget::GetType() {
	return Mask::Resource::Type::AnimationTarget;
}

void Mask::Resource::AnimationTarget::Update(Mask::Part* part, float /*time*/) {
	UNUSED_PARAMETER(part);
	return;
}

void Mask::Resource::AnimationTarget::Render(Mask::Part* part) {
	UNUSED_PARAMETER(part);
	return;
}

void Mask::Resource::AnimationTarget::SetWeights(std::map<int, float> index_to_weight_list) {

	for (int i = 0; i < m_channels.size(); i++) {
		AnimationChannel& ch = m_channels[i];
		if (ch.item != nullptr &&
			!(m_channels[i].type >= AnimationChannelType::PART_QROTATION_X &&
				m_channels[i].type <= AnimationChannelType::PART_QROTATION_W)) {
			double total_weight = 0.0;
			double total_weighted_value = 0.0;
			for (auto& e : index_to_weight_list) {
				if(channel_targets[i].find(e.first) != channel_targets[i].end())
				{
					// interpolate non-quanterions using lerp
					double f_frame = (e.first + e.second) * (m_frames_per_target - 1);
					double intpart;
					double frac = std::modf(f_frame, &intpart);
					int l_f = intpart;
					int h_f = l_f+1;
					int min_target = std::max(0, (int)(e.first * m_frames_per_target));
					int max_target = std::min((int)(ch.values.size())-1, (int)((e.first + 1) * m_frames_per_target) - 1);
					l_f = std::max(min_target, std::min(max_target, l_f));
					h_f = std::max(min_target, std::min(max_target, h_f));

					double val = ch.values[l_f] * (1.0 - frac) + ch.values[h_f] * frac;
					total_weight += e.second;
					total_weighted_value += val * e.second;
				}
			}
			// normalize the weights so sum(weights) = 1
			if(total_weight > 0.0)
				ch.item->SetAnimatableValue(total_weighted_value / total_weight, ch.type);
		}
	}

	// apply slerp for quaternions
	for (auto &ent: quaternion_channel_indices_map)
	{
		QuaternionIdx &qidx = ent.second;
		std::vector<double> weights;
		std::vector<Quaternion> values;
		for (auto& e : index_to_weight_list) {
			bool is_involved = false;
			int min_target = 0;
			int max_target = m_channels[qidx[0]].values.size() - 1;
			for (size_t k = 0; k < 4; k++)
			{
				int i = qidx[k];
				if (m_channels[i].item == nullptr) {
					is_involved = false;
					break;
				}
				if (channel_targets[i].find(e.first) != channel_targets[i].end())
					is_involved = true;
				max_target = std::min(max_target, (int)m_channels[i].values.size() - 1);
			}
			if (is_involved)
			{
				// interpolate quanterions using slerp
				double f_frame = (e.first + e.second) * (m_frames_per_target - 1);
				double intpart;
				double frac = std::modf(f_frame, &intpart);
				int l_f = intpart;
				int h_f = l_f + 1;
				min_target = std::max(min_target, (int)(e.first * m_frames_per_target));
				max_target = std::min(max_target, (int)((e.first + 1) * m_frames_per_target) - 1);
				l_f = std::max(min_target, std::min(max_target, l_f));
				h_f = std::max(min_target, std::min(max_target, h_f));

				Quaternion l_q{ m_channels[qidx[0]].values[l_f],
								m_channels[qidx[1]].values[l_f],
								m_channels[qidx[2]].values[l_f],
								m_channels[qidx[3]].values[l_f] };


				Quaternion h_q{ m_channels[qidx[0]].values[h_f],
								m_channels[qidx[1]].values[h_f],
								m_channels[qidx[2]].values[h_f],
								m_channels[qidx[3]].values[h_f] };

				Quaternion qval = slerp(l_q, h_q, frac);
				weights.push_back(e.second);
				values.push_back(qval);
			}
		}
		//
		// normalize the weights so sum(weights) = 1
		if (weights.size() > 0)
		{
			if (weights.size() == 2)
			{
				// slerp if only two values
				if (weights[0] + weights[1] > 0.00001)
				{
					Quaternion qval = slerp(values[0], values[1], weights[0] / (weights[0] + weights[1]));
					for (size_t k = 0; k < 4; k++)
					{
						AnimationChannel& ch = m_channels[qidx[k]];
						ch.item->SetAnimatableValue(qval[k], ch.type);
					}
				}
				else
				{
					// if weights are almost zero, just use the first one
					for (size_t k = 0; k < 4; k++)
					{
						AnimationChannel& ch = m_channels[qidx[k]];
						ch.item->SetAnimatableValue(values[0][k], ch.type);
					}
				}
			}
			else if (weights.size() == 1)
			{
				// just use the value
				for (size_t k = 0; k < 4; k++)
				{
					AnimationChannel& ch = m_channels[qidx[k]];
					ch.item->SetAnimatableValue(values[0][k], ch.type);
				}
			}
			else
			{
				// more than two values
				// summing all quaternions and normalizing is good enough
				// solution if the vectors are not in completely random directions
				// for more info, see:
				// C. Gramkow, On averaging rotations, International Journal of Computer Vision 42 (1–2) (2001) 7–16
				// if this doesn't work well for weighted average, consider axis-ang vector average
				Quaternion qavg{ 0,0,0,0 };
				for (size_t i = 0; i < weights.size(); i++)
				{
					qavg[0] += values[i][0] * weights[i];
					qavg[1] += values[i][1] * weights[i];
					qavg[2] += values[i][2] * weights[i];
					qavg[3] += values[i][3] * weights[i];
				}
				// now normalize
				double d = std::sqrt(qavg[0] * qavg[0] + qavg[1] * qavg[1] + qavg[2] * qavg[2] + qavg[3] * qavg[3]);
				qavg[0] /= d;
				qavg[1] /= d;
				qavg[2] /= d;
				qavg[3] /= d;

				for (size_t k = 0; k < 4; k++)
				{
					AnimationChannel& ch = m_channels[qidx[k]];
					ch.item->SetAnimatableValue(values[0][k], ch.type);
				}

			}

		}
	}

}

Mask::Resource::AnimationChannelType 
Mask::Resource::AnimationTarget::AnimationTypeFromString(const std::string& s) {

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

