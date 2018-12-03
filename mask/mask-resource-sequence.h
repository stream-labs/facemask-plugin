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
#include "mask-resource-image.h"
#include "mask.h"

namespace Mask {
	namespace Resource {

		struct SequenceInstanceData : public InstanceData {
			int		current;
			int		delta;
			float	elapsed;
			float   delay;
			bool	playback_started;
			bool	playback_ended;
			SequenceInstanceData() : current(-1), delta(1), 
				elapsed(0.0f), delay(0.0f), playback_started(false), playback_ended(false) {}

			void Reset() override {
				current = 0;
				elapsed = 0.0f;
				playback_started = false;
				playback_ended = false;
			}
			void Reset(int first, int last) {
				float a = (float)rand() / (float)RAND_MAX;
				float max = (float)last;
				float min = (float)first;
				current = (int)(a * (max - min) + min);
				elapsed = 0.0f;
				playback_started = false;
				playback_ended = false;
			}
		};

		class Sequence : public IBase {
		public:
			Sequence(Mask::MaskData* parent, std::string name, obs_data_t* data);
			virtual ~Sequence();

			virtual Type GetType() override;
			virtual void Update(Mask::Part* part, float time) override;
			virtual void Render(Mask::Part* part) override;

			std::shared_ptr<Image> GetImage() { return m_image; }
			void SetTextureMatrix(Mask::Part* part, matrix4* texmat);
			bool IsInstancePlaying();

			enum Mode : uint32_t {
				ONCE,
				REPEAT,
				BOUNCE,
				REPEAT_DELAY,
				BOUNCE_DELAY,
				SCROLL_L,
				SCROLL_R,
				SCROLL_U,
				SCROLL_D,
				SPIN_CW,
				SPIN_CCW,
			};

		protected:

			std::shared_ptr<Image>	m_image;
			int						m_rows;
			int						m_cols;
			int						m_first;
			int						m_last;
			float					m_rate;
			float					m_delay;
			Mode					m_mode;
			bool					m_randomStart;

			Mode StringToMode(std::string m);
			bool IsMultiFrameMode() {
				return (m_mode == Mask::Resource::Sequence::ONCE ||
					m_mode == Mask::Resource::Sequence::REPEAT ||
					m_mode == Mask::Resource::Sequence::BOUNCE ||
					m_mode == Mask::Resource::Sequence::REPEAT_DELAY ||
					m_mode == Mask::Resource::Sequence::BOUNCE_DELAY);
			}
			bool IsDelayMode() {
				return (m_mode == Mask::Resource::Sequence::REPEAT_DELAY ||
					m_mode == Mask::Resource::Sequence::BOUNCE_DELAY ||
					m_mode == Mask::Resource::Sequence::ONCE);
			}
			bool IsBounceMode() {
				return (m_mode == Mask::Resource::Sequence::BOUNCE ||
					m_mode == Mask::Resource::Sequence::BOUNCE_DELAY);
			}
			bool IsRepeatMode() {
				return (m_mode == Mask::Resource::Sequence::REPEAT ||
					m_mode == Mask::Resource::Sequence::REPEAT_DELAY);
			}
			bool IsTranslationMode() {
				return (m_mode == Mask::Resource::Sequence::SCROLL_L ||
					m_mode == Mask::Resource::Sequence::SCROLL_R ||
					m_mode == Mask::Resource::Sequence::SCROLL_U ||
					m_mode == Mask::Resource::Sequence::SCROLL_D);
			}
		};
	}
}
