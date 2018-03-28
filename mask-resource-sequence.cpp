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

#include "mask-resource-sequence.h"
#include "exceptions.h"
#include "plugin.h"
#include "utils.h"
extern "C" {
#pragma warning( push )
#pragma warning( disable: 4201 )
#include <libobs/util/platform.h>
#include <libobs/obs-module.h>
#include <libobs/graphics/matrix4.h>
#pragma warning( pop )
}

static const char* const S_IMAGE = "image";
static const char* const S_ROWS = "rows";
static const char* const S_COLS = "cols";
static const char* const S_FIRST = "first";
static const char* const S_LAST = "last";
static const char* const S_RATE = "rate";
static const char* const S_DELAY = "delay";
static const char* const S_MODE = "mode";
static const char* const S_RANDOMSTART = "random-start";


Mask::Resource::Sequence::Sequence(Mask::MaskData* parent, std::string name, 
	obs_data_t* data)
	: IBase(parent, name)
	, m_rows(0), m_cols(0),	m_first(0), m_last(0), m_rate(4.0f), m_delay(0.0f)
	, m_mode(Mask::Resource::Sequence::REPEAT), m_randomStart(false) {

	if (!obs_data_has_user_value(data, S_IMAGE)) {
		PLOG_ERROR("Sequence '%s' has no image.", name.c_str());
		throw std::logic_error("Sequence has no image.");
	}
	std::string imageName = obs_data_get_string(data, S_IMAGE);
	m_image = std::dynamic_pointer_cast<Image>(m_parent->GetResource(imageName));
	if (m_image == nullptr) {
		PLOG_ERROR("<Sequence '%s'> Dependency on image '%s' could not be resolved.",
			m_name.c_str(), imageName.c_str());
		throw std::logic_error("Sequence depends on non-existing image.");
	}
	if (m_image->GetType() != Type::Image) {
		PLOG_ERROR("<Sequence '%s'> Resolved mesh dependency on '%s' is not a image.",
			m_name.c_str(), imageName.c_str());
		throw std::logic_error("Image dependency of Sequence is not a image.");
	}

	if (!obs_data_has_user_value(data, S_ROWS)) {
		PLOG_ERROR("Sequence '%s' has no rows value.", name.c_str());
		throw std::logic_error("Sequence has no rows value.");
	}
	m_rows = (int)obs_data_get_int(data, S_ROWS);

	if (!obs_data_has_user_value(data, S_COLS)) {
		PLOG_ERROR("Sequence '%s' has no cols value.", name.c_str());
		throw std::logic_error("Sequence has no cols value.");
	}
	m_cols = (int)obs_data_get_int(data, S_COLS);

	if (!obs_data_has_user_value(data, S_FIRST)) {
		PLOG_ERROR("Sequence '%s' has no first value.", name.c_str());
		throw std::logic_error("Sequence has no first value.");
	}
	m_first = (int)obs_data_get_int(data, S_FIRST);

	if (!obs_data_has_user_value(data, S_LAST)) {
		PLOG_ERROR("Sequence '%s' has no last value.", name.c_str());
		throw std::logic_error("Sequence has no last value.");
	}
	m_last = (int)obs_data_get_int(data, S_LAST);

	if (obs_data_has_user_value(data, S_RATE)) {
		m_rate = (float)obs_data_get_double(data, S_RATE);
	}

	if (obs_data_has_user_value(data, S_DELAY)) {
		m_delay = (float)obs_data_get_double(data, S_DELAY);
	}

	if (obs_data_has_user_value(data, S_RANDOMSTART)) {
		m_randomStart = obs_data_get_bool(data, S_RANDOMSTART);
	}

	m_mode = Mask::Resource::Sequence::REPEAT;
	if (obs_data_has_user_value(data, S_MODE)) {
		std::string modeName = obs_data_get_string(data, S_MODE);
		m_mode = StringToMode(modeName);
	}
}

Mask::Resource::Sequence::~Sequence() {}

Mask::Resource::Type Mask::Resource::Sequence::GetType() {
	return Mask::Resource::Type::Sequence;
}

void Mask::Resource::Sequence::Update(Mask::Part* part, float time) {
	UNUSED_PARAMETER(part);
	m_parent->instanceDatas.Push(m_id);

	// get our instance data
	std::shared_ptr<SequenceInstanceData> instData =
		m_parent->instanceDatas.GetData<SequenceInstanceData>();
	if (instData->current < 0) {
		if (m_randomStart) {
			instData->Reset(m_first, m_last);
		}
		else {
			instData->Reset();
		}
	}

	// update elapsed time
	float interval = 1.0f / m_rate;
	instData->elapsed += time;

	// multiple-frame modes
	if (IsMultiFrameMode()) {

		// delay mode, and on first frame?
		if (IsDelayMode() && instData->current == 0) {
			if (instData->delay > 0.0f) {
				instData->delay -= time;
			}
			if (instData->delay > 0.0f) {
				// DELAYING PLAYBACK, BAIL
				instData->elapsed = 0.0f;
				m_parent->instanceDatas.Pop();
				return;
			}
			instData->delay = 0.0f;
		}
		// need to do a frame update?
		bool pastFirst = false;
		if (instData->elapsed >= interval) {
			// update current frame
			int numIntervals = 0;
			while (instData->elapsed > interval) {
				instData->elapsed -= interval;
				numIntervals++;
			}
			instData->current += (instData->delta * numIntervals);

			// past last?
			if (instData->current > m_last) {
				if (IsBounceMode()) {
					instData->delta = -1;
					instData->current = m_last - 1;
				}
				else if (IsRepeatMode()) {
					instData->current = m_first;
				}
				else {
					instData->current = m_last;
				}
			}
			// past first?
			if (instData->current < m_first) {
				pastFirst = true;
				if (IsBounceMode()) {
					instData->delta = 1;
					instData->current = m_first + 1;
				}
				else {
					instData->delta = 1;
					instData->current = m_first;
				}
			}
		}

		// delay mode, and on first frame?
		if (IsDelayMode() && pastFirst) {
			// set delay
			instData->delay = m_delay;
			instData->current = m_first;
		}
	}

	// single-frame transform modes
	else {
		// do nothing! everything is derived from elapsed time
	}

	m_parent->instanceDatas.Pop();
}

void Mask::Resource::Sequence::Render(Mask::Part* part) {
	UNUSED_PARAMETER(part);
}


void Mask::Resource::Sequence::SetTextureMatrix(Mask::Part* part, matrix4* texmat) {
	UNUSED_PARAMETER(part);
	matrix4_identity(texmat);

	m_parent->instanceDatas.Push(m_id);

	// get our instance data
	std::shared_ptr<SequenceInstanceData> instData =
		m_parent->instanceDatas.GetData<SequenceInstanceData>();

	int curr = instData->current;
	if (curr < 0)
		curr = 0;

	if (IsMultiFrameMode()) {
		int row = curr / m_cols;
		int col = curr % m_cols;
		texmat->x.x = 1.0f / (float)m_cols;
		texmat->y.y = 1.0f / (float)m_rows;
		texmat->x.w = (float)col / (float)m_cols;
		texmat->y.w = (float)row / (float)m_rows;
	}
	else if (IsTranslationMode()) {
		texmat->x.x = 1.0f / (float)m_cols;
		texmat->y.y = 1.0f / (float)m_rows;
		if (m_mode == Mask::Resource::Sequence::SCROLL_L)
			texmat->x.w = m_rate * instData->elapsed;
		else if (m_mode == Mask::Resource::Sequence::SCROLL_R)
			texmat->x.w = -m_rate * instData->elapsed;
		else if (m_mode == Mask::Resource::Sequence::SCROLL_U)
			texmat->y.w = m_rate * instData->elapsed;
		else if (m_mode == Mask::Resource::Sequence::SCROLL_D)
			texmat->y.w = -m_rate * instData->elapsed;
		texmat->x.w /= (float)m_cols;
		texmat->y.w /= (float)m_rows;
	}
	else {
		float angle = m_rate * instData->elapsed * M_PI * 2.0f;
		if (m_mode == Mask::Resource::Sequence::SPIN_CW)
			angle = -angle;
		float halfx = 0.5f;
		float halfy = 0.5f;
		float ra = cosf(angle);
		float rb = -sinf(angle);
		float rc = -rb;
		float rd = ra;

		texmat->x.w = halfx - (ra * halfx) - (rb * halfy);
		texmat->y.w = halfy - (rc * halfx) - (rd * halfy);
		texmat->x.x = ra;
		texmat->x.y = rb;
		texmat->y.x = rc;
		texmat->y.y = rd;
	}

	m_parent->instanceDatas.Pop();
}

Mask::Resource::Sequence::Mode Mask::Resource::Sequence::StringToMode(std::string modeName) {
	Mask::Resource::Sequence::Mode m = Mask::Resource::Sequence::ONCE;
	if (modeName == "once")
		m = Mask::Resource::Sequence::ONCE;
	else if (modeName == "bounce")
		m = Mask::Resource::Sequence::BOUNCE;
	else if (modeName == "repeat")
		m = Mask::Resource::Sequence::REPEAT;
	else if (modeName == "bounce-delay")
		m = Mask::Resource::Sequence::BOUNCE_DELAY;
	else if (modeName == "repeat-delay")
		m = Mask::Resource::Sequence::REPEAT_DELAY;
	else if (modeName == "scroll-left")
		m = Mask::Resource::Sequence::SCROLL_L;
	else if (modeName == "scroll-right")
		m = Mask::Resource::Sequence::SCROLL_R;
	else if (modeName == "scroll-up")
		m = Mask::Resource::Sequence::SCROLL_U;
	else if (modeName == "scroll-down")
		m = Mask::Resource::Sequence::SCROLL_D;
	else if (modeName == "spin-cw")
		m = Mask::Resource::Sequence::SPIN_CW;
	else if (modeName == "spin-ccw")
		m = Mask::Resource::Sequence::SPIN_CCW;
	else {
		PLOG_ERROR("Sequence '%s' has invalid mode '%s'.", m_name.c_str(), modeName.c_str());
		throw std::logic_error("Sequence has bad mode.");
	}
	return m;
}
