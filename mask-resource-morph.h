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
#include "mask-resource-animation.h"
#include "smll/landmarks.hpp"
#include "smll/TriangulationResult.hpp"
#include "MorphData.hpp"
#include <array>
#include <string>
extern "C" {
#pragma warning( push )
#pragma warning( disable: 4201 )
#include <libobs/graphics/graphics.h>
#include <libobs/graphics/vec3.h>
#pragma warning( pop )
}


namespace Mask {
	namespace Resource {

		class Morph : public IBase, public IAnimatable {
		public:
			Morph(Mask::MaskData* parent, std::string name, obs_data_t* data);
			Morph(Mask::MaskData* parent, std::string name); // constructs an "identity" morph
			virtual ~Morph();

			virtual Type GetType() override;
			virtual void Update(Mask::Part* part, float time) override;
			virtual void Render(Mask::Part* part) override;
			virtual bool IsDepthOnly() override;

			const smll::MorphData&	GetMorphData() { return m_morphData; }

			void RenderMorphVideo(gs_texture* vidtex, 
				const smll::TriangulationResult& trires);

			// IAnimatable
			void SetAnimatableValue(float v,
				Resource::AnimationChannelType act) override;

		protected:

			smll::MorphData		m_morphData;
			gs_effect_t*		m_drawEffect;

			std::array<gs_indexbuffer_t*, smll::NUM_FACE_AREAS> faceIndexBuffers;
			void	MakeFaceIndexBuffers();
		};
	}
}
