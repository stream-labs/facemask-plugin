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

namespace Mask {

	std::shared_ptr<Signal> Signal::Create(MaskData *mask_data, json signal_info) {
		std::string type = signal_info["type"].get<std::string>();
		if (type == "timeout")
		{
			// create timeout event
		}
	}

	Signal::Data TimeoutSignal::check(float time_delta, smll::TriangulationResult *triangulation) {
		elapsed += time_delta;

		if (elapsed >= duration)
		{
			if (repeat) elapsed -= (int)(elapsed / duration) * duration;
			return std::make_pair(true,json());
		}

		return std::make_pair(false,json());
	}

	EventSystem::EventSystem(Mask::MaskData* parent, obs_data_t* data) {
		this->parent = parent;
		json events = json::parse(obs_data_get_json(data));
		state = events["state"];
		for (const auto &itm: events["items"])
		{
			SignalList signals;
			EventHandlerList handlers;
			for (const auto &sig : itm["signals"])
				signals.push_back(Signal::Create(sig));

			for (const auto& h : itm["handlers"])
				handlers.push_back(EventHandler(parent,h));

			event_list.push_back(std::make_pair(signals, handlers));
		}
	}

	void EventSystem::Tick(float time_delta, smll::TriangulationResult *triangulation) {
		for (auto& ev : event_list)
		{
			bool is_triggered = false;
			json signal_data_list = json::array();
			for (auto &signal : ev.first)
			{
				// use of `current_triggered` var forces
				// `signal->is_on()` to be called on every tick
				Signal::Data current_data = signal->is_on(time_delta, triangulation);
				is_triggered = is_triggered || current_data.first;
				if (current_data.first)
					signal_data_list.push_back(current_data.second);
			}
			if (is_triggered)
			{
				for (EventHandler &handler : ev.second)
					handler.handle(signal_data_list, time_delta, triangulation);
			}
		}
	}

}
