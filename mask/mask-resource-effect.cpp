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

#include "mask-resource-effect.h"
#include "exceptions.h"
#include "plugin.h"
#include "utils.h"

Mask::Resource::Effect::Effect(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name) {
	const char* const S_DATA = "data";
	if (!obs_data_has_user_value(data, S_DATA)) {
		PLOG_ERROR("Effect '%s' has no data.", name.c_str());
		throw std::logic_error("Effect has no data.");
	}
	const char* base64data = obs_data_get_string(data, S_DATA);
	if (base64data[0] == '\0') {
		PLOG_ERROR("Effect '%s' has empty data.", name.c_str());
		throw std::logic_error("Effect has empty data.");
	}

	// write to temp file
	m_filename = Utils::Base64ToTempFile(base64data);
	m_filenameIsTemp = true;
}

Mask::Resource::Effect::Effect(Mask::MaskData* parent, std::string name, std::string filename)
	: IBase(parent, name) {

	m_filename = filename;
	m_filenameIsTemp = false;
}


Mask::Resource::Effect::~Effect() {}

Mask::Resource::Type Mask::Resource::Effect::GetType() {
	return Mask::Resource::Type::Effect;
}

void Mask::Resource::Effect::Update(Mask::Part* part, float time) {
	UNUSED_PARAMETER(part);
	UNUSED_PARAMETER(time);
	return;
}

void Mask::Resource::Effect::Render(Mask::Part* part) {
	UNUSED_PARAMETER(part);
	if (m_Effect == nullptr) {
		m_Effect = std::make_shared<GS::Effect>(m_filename);
		if (m_filenameIsTemp) {
			Utils::DeleteTempFile(m_filename);
		}
		m_filename.clear();
	}

	return;
}
