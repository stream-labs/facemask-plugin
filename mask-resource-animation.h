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
			PART_POSITION_X,
			PART_POSITION_Y,
			PART_POSITION_Z,
			PART_QROTATION_X,
			PART_QROTATION_Y,
			PART_QROTATION_Z,
			PART_QROTATION_W,
			PART_SCALE_X,
			PART_SCALE_Y,
			PART_SCALE_Z,
		};

		struct AnimationInstanceData : public InstanceData {
			float	elapsed;
			AnimationInstanceData() : elapsed(0.0f) {}
		};

		struct AnimationChannel {
			std::string					name;
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

		protected:
			float							m_duration;
			float							m_fps;
			std::vector<AnimationChannel>	m_channels;

			AnimationChannelType AnimationTypeFromString(const std::string& s);
			AnimationBehaviour AnimationBehaviourFromString(const std::string& s);
		};
	}
}
