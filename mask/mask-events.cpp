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

	std::shared_ptr<EventHandler> EventHandler::Create(MaskData* mask_data, json handler_data, const json& initial_context) {

		if (handler_data["type"].get<std::string>() == "expression")
		{
			//return std::make_shared<ExpressionHandler>(handler_data["expression"].get<std::string>(), initial_context);
		}
		else if (handler_data["type"].get<std::string>() == "animation-target-weight-list")
		{
			return std::make_shared<AnimationTargetHandler>(mask_data, handler_data, initial_context);
		}
		throw std::runtime_error("Unsupported event handler");
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

	//void EventHandler::handle(json signal_data_list, float time_delta, smll::TriangulationResult* result, json& context) {
	//	if (handler_data["type"].get<std::string>() == "expression")
	//	{
	//		std::string var = exp->get_assign_variable();
	//		if (var.length() > 0)
	//		{
	//			if (context[var].is_number_integer())
	//				context[var] = exp->eval_to_int(context).value;
	//			else if (context[var].is_number_float())
	//				context[var] = exp->eval_to_double(context).value;
	//		}
	//		else
	//		{
	//			// NOTE at this point, such a expression has no side effect
	//			//      for now just evaluate for no reason
	//			exp->eval<double>(context);
	//		}
	//	}
	//	else if (handler_data["type"].get<std::string>() == "animation-target-weight-list")
	//	{

	//		mask_data->SetAnimationTargetWeight(handler_data["animation-target-list"].get<std::string>(),
	//			handler_data["target-index"].get<int>(),
	//			exp->eval_to_double(context).value);
	//	}
	//}

	void AnimationTargetHandler::handle(json signal_data_list, float time_delta, smll::TriangulationResult* result, json& context) {
		std::map<std::string, std::vector<std::pair<int, double>>> weights;
		for (const auto& target : targets)
		{
			if (weights.find(target.animation_target_list) == weights.end())
			{
				weights[target.animation_target_list] = std::vector<std::pair<int, double>>();
			}
			weights[target.animation_target_list].push_back(
				std::make_pair(target.target_index, target.expression->eval_to_double(context).value)
			);
		}
		for (const auto& ent : weights)
		{
			mask_data->SetAnimationTargetWeights(ent.first, { ent.second.begin(), ent.second.end() });
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
				handlers.push_back(EventHandler::Create(parent,h,state));

			event_list.push_back(std::make_pair(signals, handlers));
		}
#ifdef EVENT_SYSTEM_GRAPH_LOG
		const char *log_path = os_get_config_path_ptr("slobs-client/node-obs/logs");
		std::string path = std::string(log_path) + "/event_system_graph_log.csv";
		graph_log = std::ofstream(path, std::ios::out | std::ios::trunc);
		//graph_log << "TIMESTAMP" << ", " <<
		//	"MOUTH_INNER_WIDTH" << ", " <<
		//	"MOUTH_INNER_WIDTH_NORM" << ", " <<
		//	"MOUTH_OUTER_WIDTH" << ", " <<
		//	"MOUTH_OUTER_WIDTH_NORM" << ", " <<
		//	"MOUTH_INNER_HEIGHT" << ", " <<
		//	"MOUTH_INNER_HEIGHT_NORM" << ", " <<
		//	"MOUTH_OUTER_HEIGHT" << ", " <<
		//	"MOUTH_OUTER_HEIGHT_NORM" << std::endl;
		graph_log << "TIMESTAMP, HEIGHT, SMOOTH_HEIGHT" << std::endl;
#endif
	}

	void EventSystem::Tick(float time_delta, smll::TriangulationResult *triangulation) {

		// if we are collecting the last element in period
		// we are ready to take moving average
		if (run == FILTER_PERIOD - 1) filter_active = true;

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
				for (auto &handler : ev.second)
					handler->handle(signal_data_list, time_delta, triangulation, current_full_state);
			}
		}
		run = (run + 1) % FILTER_PERIOD;
	}

	float EventSystem::smooth(std::string name, float value, float time) {
		if (value_history_map.find(name) == value_history_map.end())
			value_history_map[name] = std::array<float, FILTER_PERIOD>();

		value_history_map[name][run] = value;
		double smooth_val = 0.0;

		if (filter_active)
		{
			std::array<double, FILTER_PERIOD-1> history;
			std::array<double, FILTER_PERIOD-1> timestamps;
			double time_begin;

			for (size_t i = 1; i < FILTER_PERIOD; i++)
			{
				int j = (run + i + 1) % FILTER_PERIOD;
				history[i-1] = (value_history_map[name][((j - 1) + FILTER_PERIOD) % FILTER_PERIOD] +
					         value_history_map[name][j] +
					         value_history_map[name][(j + 1) % FILTER_PERIOD]) / 3.0;
				history[i - 1] = ((FILTER_PERIOD - i - 1) * history[i - 1] + (i - 1) * value_history_map[name][j]) / (FILTER_PERIOD-2);
				timestamps[i - 1] = value_history_map["TIME"][j];
			}

			time_begin = value_history_map["TIME"][(run + 1) % FILTER_PERIOD];
			std::for_each(timestamps.begin(), timestamps.end(), [time_begin](double& d) { d -= time_begin; });

			const int n = history.size();
			auto &x = timestamps;
			auto &y = history;
			const double s_x = std::accumulate(x.begin(), x.end(), 0.0);
			const double s_y = std::accumulate(y.begin(), y.end(), 0.0);
			const double s_xx = std::inner_product(x.begin(), x.end(), x.begin(), 0.0);
			const double s_xy = std::inner_product(x.begin(), x.end(), y.begin(), 0.0);
			const double offset = (s_y * s_xx - s_x * s_xy) / (n * s_xx - s_x * s_x);
			const double slope = (n * s_xy - s_x * s_y) / (n * s_xx - s_x * s_x);

			smooth_val = offset + slope * timestamps[FILTER_PERIOD - 2];
		}
		else
		{
			for (size_t i = 0; i <= run; i++)
			{
				smooth_val += value_history_map[name][i];
			}
			smooth_val /= run + 1;
		}
			

		return smooth_val;
	}

	json EventSystem::get_full_state(float time_delta, smll::TriangulationResult* triangulation) {
		json full_state = state;

		total_time += time_delta;
		full_state["TIME"] = total_time;
		if (value_history_map.find("TIME") == value_history_map.end())
			value_history_map["TIME"] = std::array<float, FILTER_PERIOD>();
		value_history_map["TIME"][run] = total_time;

		for (size_t i = 0; i < smll::NUM_FACIAL_LANDMARKS; i++)
		{
			full_state[std::string("LANDMARK") + std::to_string(i) + "_X"] = triangulation->points[i].x;
			full_state[std::string("LANDMARK") + std::to_string(i) + "_Y"] = triangulation->points[i].y;
		}

		auto dist = [](cv::Point2f p1, cv::Point2f p2) {
			return cv::norm(p1 - p2);
		};


#ifdef EVENT_SYSTEM_GRAPH_LOG
		graph_log << total_time << ", ";
#endif

		full_state["FACE_HEIGHT"] = dist(triangulation->points[smll::HEAD_6],triangulation->points[smll::JAW_9]);
		//full_state["FACE_HEIGHT"] = smooth("FACE_HEIGHT", full_state["FACE_HEIGHT"].get<float>(), total_time);
		full_state["FACE_HEIGHT"] = full_state["FACE_HEIGHT"].get<float>();

		full_state["FACE_WIDTH"] = dist(triangulation->points[smll::JAW_1],triangulation->points[smll::JAW_17]);
		//full_state["FACE_WIDTH"] = smooth("FACE_WIDTH", full_state["FACE_WIDTH"].get<float>(), total_time);
		full_state["FACE_WIDTH"] = full_state["FACE_WIDTH"].get<float>();


		full_state["MOUTH_OUTER_WIDTH"] = dist(triangulation->points[smll::RIGHT_MOUTH_CORNER],triangulation->points[smll::LEFT_MOUTH_CORNER]);
		full_state["MOUTH_OUTER_WIDTH"] = smooth("MOUTH_OUTER_WIDTH",full_state["MOUTH_OUTER_WIDTH"].get<float>(), total_time);

		full_state["MOUTH_OUTER_HEIGHT"] = dist(triangulation->points[smll::MOUTH_OUTER_4],triangulation->points[smll::MOUTH_OUTER_10]);
		full_state["MOUTH_OUTER_HEIGHT"] = smooth("MOUTH_OUTER_HEIGHT", full_state["MOUTH_OUTER_HEIGHT"].get<float>(), total_time);


		full_state["MOUTH_INNER_WIDTH"] = dist(triangulation->points[smll::MOUTH_INNER_1],triangulation->points[smll::MOUTH_INNER_5]);
		full_state["MOUTH_INNER_WIDTH"] = smooth("MOUTH_INNER_WIDTH", full_state["MOUTH_INNER_WIDTH"].get<float>(), total_time);

		full_state["MOUTH_INNER_HEIGHT"] = dist(triangulation->points[smll::MOUTH_INNER_7],triangulation->points[smll::MOUTH_INNER_3]);
#ifdef EVENT_SYSTEM_GRAPH_LOG
		graph_log << full_state["MOUTH_INNER_HEIGHT"].get<float>() / full_state["FACE_HEIGHT"].get<float>() << ", ";
#endif
		full_state["MOUTH_INNER_HEIGHT"] = smooth("MOUTH_INNER_HEIGHT", full_state["MOUTH_INNER_HEIGHT"].get<float>(), total_time);
#ifdef EVENT_SYSTEM_GRAPH_LOG
		graph_log << full_state["MOUTH_INNER_HEIGHT"].get<float>() / full_state["FACE_HEIGHT"].get<float>() << std::endl;
#endif

//#ifdef EVENT_SYSTEM_GRAPH_LOG
//		graph_log << total_time << ", " <<
//			         full_state["MOUTH_INNER_WIDTH"].get<float>() << ", " <<
//			         full_state["MOUTH_INNER_WIDTH"].get<float>() / full_state["FACE_WIDTH"].get<float>() << ", " <<
//		             full_state["MOUTH_OUTER_WIDTH"].get<float>() << ", " <<
//			         full_state["MOUTH_OUTER_WIDTH"].get<float>() / full_state["FACE_WIDTH"].get<float>() << ", " <<
//			         // mouth height columns
//					 full_state["MOUTH_INNER_HEIGHT"].get<float>() << ", " <<
//					 full_state["MOUTH_INNER_HEIGHT"].get<float>() / full_state["FACE_HEIGHT"].get<float>() << ", " <<
//					 full_state["MOUTH_OUTER_HEIGHT"].get<float>() << ", " <<
//					 full_state["MOUTH_OUTER_HEIGHT"].get<float>() / full_state["FACE_HEIGHT"].get<float>() << ", " << std::endl;
//#endif


		return full_state;
	}

}
