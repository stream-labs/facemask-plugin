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
#include "gs/gs-effect.h"
#include <sstream>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/util/platform.h>
	#include <libobs/obs-module.h>
	#pragma warning( pop )
}

namespace Mask {
	namespace Resource {
		class Effect : public IBase {
		public:
			Effect(Mask::MaskData* parent, std::string name, obs_data_t* data);
			Effect(Mask::MaskData* parent, std::string name, std::string filename);
			virtual ~Effect();

			virtual Type GetType() override;
			virtual std::shared_ptr<GS::Effect> GetEffect() { return m_Effect; }
			virtual void Update(Mask::Part* part, float time) override;
			virtual void Render(Mask::Part* part) override;

			void SetActiveTextures(const std::vector<std::string> &textures) { m_active_textures = textures; }

		protected:
			std::shared_ptr<GS::Effect> m_Effect;

			// for delayed gs creation
			std::string		m_filename;
			bool			m_filenameIsTemp;

			// for dynamic shader creation
			std::vector<std::string> m_active_textures;
		};
	}
}
