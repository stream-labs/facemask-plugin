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
#pragma once
#include "mask-resource.h"
#include "mask-instance-data.h"
#include <vector>

namespace Mask {

	namespace Resource {

		enum AnimationBehaviour : uint32_t {
			CONSTANT,
			LINEAR,
			REPEAT,
		};

		enum AnimationChannelType : uint32_t {
			INVALID_TYPE,

			// part channels
			PART_CHANNEL_FIRST,
			PART_POSITION_X = PART_CHANNEL_FIRST,
			PART_POSITION_Y,
			PART_POSITION_Z,
			PART_QROTATION_X,
			PART_QROTATION_Y,
			PART_QROTATION_Z,
			PART_QROTATION_W,
			PART_SCALE_X,
			PART_SCALE_Y,
			PART_SCALE_Z,
			PART_CHANNEL_LAST,

			// morph channels (there are 68 xyz channels)
			MORPH_CHANNEL_FIRST,
			MORPH_LANDMARK_0_X = MORPH_CHANNEL_FIRST,
			MORPH_LANDMARK_0_Y,
			MORPH_LANDMARK_0_Z,
			MORPH_LANDMARK_67_X = MORPH_LANDMARK_0_X + (67 * 3),
			MORPH_LANDMARK_67_Y = MORPH_LANDMARK_0_Y + (67 * 3),
			MORPH_LANDMARK_67_Z = MORPH_LANDMARK_0_Z + (67 * 3),
			MORPH_CHANNEL_LAST,
		};

		class IAnimatable {
		public:
			virtual void SetAnimatableValue(float v, AnimationChannelType act) = 0;
		};

		struct AnimationInstanceData : public InstanceData {
			float	elapsed;
			AnimationInstanceData() : elapsed(0.0f) {}
		};

		struct AnimationChannel {
			std::shared_ptr<IAnimatable>	item;
			AnimationChannelType		type;
			AnimationBehaviour			preState;
			AnimationBehaviour			postState;
			std::vector<float>			values;

			float	GetValue(float time);
		};


		class Animation : public IBase {
		public:
			Animation(Mask::MaskData* parent, std::string name, obs_data_t* data);
			virtual ~Animation();

			virtual Type GetType() override;
			virtual void Update(Mask::Part* part, float time) override;
			virtual void Render(Mask::Part* part) override;

			void Rewind();

		protected:
			float							m_duration;
			float							m_fps;
			std::vector<AnimationChannel>	m_channels;

			AnimationChannelType AnimationTypeFromString(const std::string& s);
			AnimationBehaviour AnimationBehaviourFromString(const std::string& s);
		};
	}
}
