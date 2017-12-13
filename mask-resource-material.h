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
#include "mask-resource-effect.h"
#include "mask-resource-image.h"
#include "gs-effect.h"
#include <string>
#include <vector>
#include <array>

namespace Mask {
	namespace Resource {
		class Material : public IBase {
			public:
			Material(Mask::MaskData* parent, std::string name, obs_data_t* data);
			virtual ~Material();

			virtual Mask::Resource::Type GetType() override;
			virtual void Update(Mask::Part* part, float time) override;
			virtual void Render(Mask::Part* part) override;

			bool Loop(Mask::Part* part);

			bool IsDepthOnly() override { return m_depthOnly; }
			bool IsOpaque() override { return m_opaque; }

			private:
			struct Parameter {
				GS::EffectParameter::Type type;
				union {
					matrix4 matrix;
					int32_t intArray[4];
					int32_t intValue;
					float_t floatArray[4];
					float_t floatValue;
				};
			};

			protected:
			std::shared_ptr<Effect> m_effect;
			std::string m_technique;

			std::map<std::string, Parameter> m_parameters;
			std::map<std::string, std::shared_ptr<IBase>> m_imageParameters;
			std::map<std::string, std::vector<int32_t>> m_integerArrayParameters;
			std::map<std::string, std::vector<float_t>> m_floatArrayParameters;

			bool m_looping;
			gs_technique_t* m_currentTechnique;
			gs_samplerstate_t* m_samplerState;
			size_t m_techniquePasses, m_techniquePass;

			gs_address_mode m_wrapU;
			gs_address_mode m_wrapV;
			gs_address_mode m_wrapW;
			gs_cull_mode m_culling;
			gs_depth_test m_depthTest;
			gs_sample_filter m_filter;
			std::array<size_t, 8> m_lightIds;
			bool m_depthOnly;
			bool m_opaque;

			gs_address_mode StringToAddressMode(std::string s);
			void SetLightingParameters();
		};
	}
}
