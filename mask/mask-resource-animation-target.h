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

			using Quaternion = std::array<double, 4>;
			using QuaternionIdx = std::array<size_t, 4>;
			std::map<std::shared_ptr<IAnimatable>, QuaternionIdx> quaternion_channel_indices_map;

			AnimationChannelType AnimationTypeFromString(const std::string& s);

			/* doesn't look it's working right
			Quaternion slerp(const Quaternion &a, const Quaternion &b, double t) {
				Quaternion r;
				double t_ = 1 - t;
				double Wa, Wb;
				double theta = acos(a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3]);
				double sn = sin(theta);
				Wa = sin(t_ * theta) / sn;
				Wb = sin(t * theta) / sn;
				r[0] = Wa * a[0] + Wb * b[0];
				r[1] = Wa * a[1] + Wb * b[1];
				r[2] = Wa * a[2] + Wb * b[2];
				r[3] = Wa * a[3] + Wb * b[3];
				double d = std::sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2] + r[3] * r[3]);
				r[0] /= d;
				r[1] /= d;
				r[2] /= d;
				r[3] /= d;
				return r;
			}
			*/
			Quaternion slerp(Quaternion v0, Quaternion v1, double t) {
				// Only unit quaternions are valid rotations.
				// Normalize to avoid undefined behavior.
				v0.normalize();
				v1.normalize();

				// Compute the cosine of the angle between the two vectors.
				double dot = dot_product(v0, v1);

				// If the dot product is negative, slerp won't take
				// the shorter path. Note that v1 and -v1 are equivalent when
				// the negation is applied to all four components. Fix by 
				// reversing one quaternion.
				if (dot < 0.0f) {
					v1 = -v1;
					dot = -dot;
				}

				const double DOT_THRESHOLD = 0.9995;
				if (dot > DOT_THRESHOLD) {
					// If the inputs are too close for comfort, linearly interpolate
					// and normalize the result.

					Quaternion result = v0 + t * (v1 - v0);
					result.normalize();
					return result;
				}

				// Since dot is in range [0, DOT_THRESHOLD], acos is safe
				double theta_0 = acos(dot);        // theta_0 = angle between input vectors
				double theta = theta_0 * t;          // theta = angle between v0 and result
				double sin_theta = sin(theta);     // compute this value only once
				double sin_theta_0 = sin(theta_0); // compute this value only once

				double s0 = cos(theta) - dot * sin_theta / sin_theta_0;  // == sin(theta_0 - theta) / sin(theta_0)
				double s1 = sin_theta / sin_theta_0;

				return (s0 * v0) + (s1 * v1);
			}
		};
	}
}
