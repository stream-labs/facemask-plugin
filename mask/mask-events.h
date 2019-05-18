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

#include "mask.h"
#include "smll/TriangulationResult.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <map>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/obs-data.h>
	#pragma warning( pop )
}

namespace Mask {

	using json = nlohmann::json;

	class Signal {
	public:
		enum class EdgeType {
			ACTIVE,
			INACTIVE,
			RISING,
			FALLING
		};
		using Data = std::pair<bool, json>;
		EdgeType edge_type_from_str(std::string edge_type_str)
		{
			if (edge_type_str == "active")   return EdgeType::ACTIVE;
			if (edge_type_str == "inactive") return EdgeType::INACTIVE;
			if (edge_type_str == "rising")   return EdgeType::RISING;
			if (edge_type_str == "falling")  return EdgeType::FALLING;

			throw std::runtime_error("Unsupported edge type for signal");
		}

		static std::shared_ptr<Signal> Create(MaskData *mask_data, json signal_info);

		Signal(EdgeType edge_type = EdgeType::RISING) {
			previous_state = false;
			this->edge_type = edge_type;
		}

		Data is_on(float time_delta, smll::TriangulationResult *triangulation) {
			std::pair<EdgeType, json> state = get_state(time_delta, triangulation);
			// if we are looking for active signal state
			// both rising and active types are acceptable
			if (edge_type == EdgeType::ACTIVE)
				return std::make_pair(state.first == EdgeType::ACTIVE ||
					state.first == EdgeType::RISING,
					state.second);
			else if (edge_type == EdgeType::INACTIVE)
				return std::make_pair(state.first == EdgeType::INACTIVE ||
					state.first == EdgeType::FALLING,
					state.second);
			else
				return std::make_pair(state.first == edge_type, state.second);
		}

		std::pair<EdgeType,json> get_state(float time_delta, smll::TriangulationResult *triangulation) {
			Data new_data = check(time_delta, triangulation);
			EdgeType state = EdgeType::INACTIVE;
			if (new_data.first && !previous_state)
				state = EdgeType::RISING;
			else if (!new_data.first && previous_state)
				state = EdgeType::FALLING;
			else if (new_data.first)
				state = EdgeType::ACTIVE;
			else if (!new_data.first)
				state = EdgeType::INACTIVE;
			previous_state = new_data.first;
			return std::make_pair(state,new_data.second);
		}
	private:
		bool previous_state;
		EdgeType edge_type;
		virtual Data check(float time_delta, smll::TriangulationResult *triangulation) = 0;
	};
	class TimeoutSignal : public Signal {
	private:
		float duration; // in seconds
		bool repeat;
		float elapsed;
	public:
		TimeoutSignal(json info):Signal(edge_type_from_str(info["type"].get<std::string>()))
		{
			duration = info.value("duration", 1.0/60.0); // default to 60 fps
			repeat = info.value("repeat", true);
			elapsed = 0.0;
		}
		virtual Data check(float time_delta, smll::TriangulationResult *triangulation) override;
	};
	class EventHandler {
		MaskData* mask_data;
	public:
		EventHandler(MaskData *mask_data, json handler_data) {
			this->mask_data = mask_data;
		}
		void handle(json signal_data_list, float time_delta, smll::TriangulationResult *result) {

		}
	};

	class EventSystem {
	public:
		using SignalList = std::vector<std::shared_ptr<Signal>>;
		using EventHandlerList = std::vector<EventHandler>;
		using Event = std::pair<SignalList, EventHandlerList>;

		json state;
		std::vector<Event> event_list;

		EventSystem(Mask::MaskData* parent, obs_data_t* data);
		void Tick(float time_delta, smll::TriangulationResult *triangulation);
	private:
		MaskData *parent;
	};
}
