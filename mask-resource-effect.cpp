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
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/util/platform.h>
	#include <libobs/obs-module.h>
	#pragma warning( pop )
}
#include <sstream>

static const char* const S_DATA = "data";

Mask::Resource::Effect::Effect(Mask::MaskData* parent, std::string name, obs_data_t* data)
	: IBase(parent, name) {

	if (!obs_data_has_user_value(data, S_DATA)) {
		PLOG_ERROR("Effect '%s' has no data.", name.c_str());
		throw std::logic_error("Effect has no data.");
	}
	const char* base64data = obs_data_get_string(data, S_DATA);
	if (base64data[0] == '\0') {
		PLOG_ERROR("Effect '%s' has empty data.", name.c_str());
		throw std::logic_error("Effect has empty data.");
	}

	// This code works
	//
	const char* tempFile = Utils::Base64ToTempFile(base64data);
	m_Effect = std::make_shared<GS::Effect>(tempFile);
	Utils::DeleteTempFile(tempFile);

	// This code does not work
	// TODO: make it work! hitting the file system sucks!
	//
	//std::vector<uint8_t> decodedData = base64_decodeZ(base64data,
	//	strlen(base64data), base64_mime_alphabet);
	//m_Effect = std::make_shared<GS::Effect>(std::string(
	//	(char*)decodedData.data()), m_name);
}

Mask::Resource::Effect::Effect(Mask::MaskData* parent, std::string name, std::string filename)
	: IBase(parent, name) {

	m_Effect = std::make_shared<GS::Effect>(filename);
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
	return;
}
