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
#include "mask-resource-animation.h"
#include <vector>
extern "C" {
#pragma warning( push )
#pragma warning( disable: 4201 )
#include <libobs/util/platform.h>
#include <libobs/obs-module.h>
#pragma warning( pop )
}

namespace Mask {

	namespace Resource {

		class AnimationTarget : public IBase {
		public:
			AnimationTarget(Mask::MaskData* parent, std::string name, obs_data_t* data);
			virtual ~AnimationTarget();

			virtual Type GetType() override;
			virtual void Update(Mask::Part* part, float time) override;
			virtual void Render(Mask::Part* part) override;

			void SetWeights(std::map<int, float> index_to_weight_list);
			
		private:
			const char* const S_DURATION = "duration";
			const char* const S_FPT = "frames-per-target";
			const char* const S_CHANNELS = "channels";
			const char* const S_NAME = "name";
			const char* const S_TYPE = "type";
			const char* const S_VALUES = "values";

		protected:
			float							m_frames_count;
			float							m_frames_per_target;
			std::vector<AnimationChannel>	m_channels;
			std::vector<std::unordered_set<int>> channel_targets;

			AnimationChannelType AnimationTypeFromString(const std::string& s);
		};
	}
}
