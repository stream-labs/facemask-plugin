/*
 * Face Masks for SlOBS
 * Copyright (C) 2019 General Workings Inc
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

#include "mask-events.h"

#include "mask.h"
#include "smll/landmarks.hpp"

namespace Mask {

	std::shared_ptr<Signal> Signal::Create(MaskData *mask_data, json signal_info, const json &initial_context) {
		std::string type = signal_info["type"].get<std::string>();
		if (type == "timeout")
		{
			// create timeout event
			return std::make_shared<TimeoutSignal>(signal_info);
		}
	}

	Signal::Data TimeoutSignal::check(float time_delta, smll::TriangulationResult * /* not used */, const json & /*context*/) {
		elapsed += time_delta;

		if (elapsed >= duration)
		{
			if (repeat) elapsed -= (int)(elapsed / duration) * duration;
			return std::make_pair(true,json());
		}

		return std::make_pair(false,json());
	}

	void EventHandler::handle(json signal_data_list, float time_delta, smll::TriangulationResult* result, json& context) {
		if (handler_data["type"].get<std::string>() == "expression")
		{
			std::string var = exp->get_assign_variable();
			if (var.length() > 0)
			{
				if (context[var].is_number_integer())
					context[var] = exp->eval_to_int(context).value;
				else if (context[var].is_number_float())
					context[var] = exp->eval_to_double(context).value;
			}
			else
			{
				// NOTE at this point, such a expression has no side effect
				//      for now just evaluate for no reason
				exp->eval<double>(context);
			}
		}
		else if (handler_data["type"].get<std::string>() == "assign-animation-target-weight")
		{
			mask_data->SetAnimationTargetWeight(handler_data["animation-target-list"].get<std::string>(),
				handler_data["target-index"].get<int>(),
				exp->eval_to_double(context).value);
		}
	}


	EventSystem::EventSystem(MaskData* parent, obs_data_t* data) {
		this->parent = parent;
		json events = json::parse(obs_data_get_json(data));
		state = events.value("state",json::object());
		for (const auto &itm: events["items"])
		{
			SignalList signals;
			EventHandlerList handlers;
			for (const auto &sig : itm["signals"])
				signals.push_back(Signal::Create(parent,sig,state));

			for (const auto& h : itm["handlers"])
				handlers.push_back(EventHandler(parent,h,state));

			event_list.push_back(std::make_pair(signals, handlers));
		}
	}

	void EventSystem::Tick(float time_delta, smll::TriangulationResult *triangulation) {
		json current_full_state = get_full_state(time_delta, triangulation);
		for (auto& ev : event_list)
		{
			bool is_triggered = false;
			json signal_data_list = json::array();
			for (auto &signal : ev.first)
			{
				// use of `current_triggered` var forces
				// `signal->is_on()` to be called on every tick
				Signal::Data current_data = signal->is_on(time_delta, triangulation, current_full_state);
				is_triggered = is_triggered || current_data.first;
				if (current_data.first)
					signal_data_list.push_back(current_data.second);
			}
			if (is_triggered)
			{
				for (EventHandler &handler : ev.second)
					handler.handle(signal_data_list, time_delta, triangulation, current_full_state);
			}
		}
	}

	json EventSystem::get_full_state(float time_delta, smll::TriangulationResult* triangulation) {
		json full_state = state;

		for (size_t i = 0; i < smll::NUM_FACIAL_LANDMARKS; i++)
		{
			full_state[std::string("LANDMARK") + std::to_string(i) + "_X"] = triangulation->points[i].x;
			full_state[std::string("LANDMARK") + std::to_string(i) + "_Y"] = triangulation->points[i].y;
		}

		auto dist = [](cv::Point2f p1, cv::Point2f p2) {
			return cv::norm(p1 - p2);
		};

		full_state["MOUTH_OUTER_WIDTH"] = dist(triangulation->points[smll::RIGHT_MOUTH_CORNER],
			                        triangulation->points[smll::LEFT_MOUTH_CORNER]);
		full_state["MOUTH_OUTER_HEIGHT"] = dist(triangulation->points[smll::MOUTH_OUTER_4],
			                        triangulation->points[smll::MOUTH_OUTER_10]);
		full_state["FACE_HEIGHT"] = dist(triangulation->points[smll::HEAD_6],
			triangulation->points[smll::JAW_9]);
		full_state["FACE_WIDTH"] = dist(triangulation->points[smll::JAW_1],
			triangulation->points[smll::JAW_17]);

		blog(LOG_DEBUG, "Mouth Width: %f", full_state["MOUTH_OUTER_WIDTH"].get<float>());
		blog(LOG_DEBUG, "Mouth Height: %f", full_state["MOUTH_OUTER_HEIGHT"].get<float>());
		blog(LOG_DEBUG, "Face Width: %f", full_state["FACE_WIDTH"].get<float>());
		blog(LOG_DEBUG, "Face Height: %f", full_state["FACE_HEIGHT"].get<float>());
		blog(LOG_DEBUG, "Mouth Width (Norm): %f", full_state["MOUTH_OUTER_WIDTH"].get<float>()/full_state["FACE_WIDTH"].get<float>());
		blog(LOG_DEBUG, "Mouth Height (Norm): %f", full_state["MOUTH_OUTER_HEIGHT"].get<float>()/full_state["FACE_HEIGHT"].get<float>());
		return full_state;
	}

}
