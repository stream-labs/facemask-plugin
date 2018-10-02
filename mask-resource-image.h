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
#include "gs-texture.h"
#include "mask.h"
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/util/platform.h>
	#include <libobs/obs-module.h>
	#pragma warning( pop )
}
#include <sstream>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <thread>

namespace Mask {
	namespace Resource {
		class Image : public IBase {
		public:
			Image(Mask::MaskData* parent, std::string name, obs_data_t* data);
			Image(Mask::MaskData* parent, std::string name, std::string filename);
			virtual ~Image();

			virtual Type GetType() override;
			virtual std::shared_ptr<GS::Texture> GetTexture() { return m_Texture; }
			virtual void Update(Mask::Part* part, float time) override;
			virtual void Render(Mask::Part* part) override;

			void SwapTexture(gs_texture* tex, bool mineToDestroy = false) {
				m_Texture = std::make_shared<GS::Texture>(tex, mineToDestroy);
			}

		private:
			const char* const S_DATA = "data";
			const char* const S_WIDTH = "width";
			const char* const S_HEIGHT = "height";
			const char* const S_MIP_LEVELS = "mip-levels";
			const char* const S_MIP_DATA = "mip-data-%d";
			const char* const S_BPP = "bpp";
			

		protected:
			std::shared_ptr<GS::Texture> m_Texture;

			// delayed gs creation
			int				m_width, m_height;
			gs_color_format m_fmt;
			std::string		m_tempFile;
			std::vector<std::vector<uint8_t>> m_decoded_mips;
		};
	}
}
