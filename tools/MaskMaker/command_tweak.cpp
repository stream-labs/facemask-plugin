/*
*
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
#include "stdafx.h"
#include "utils.h"
#include "command_tweak.h"


void command_tweak(Args& args) {

	// load json file
	json j = args.loadJsonFile(args.filename);
	args.jptr = &j;

	for (auto it = args.kvpairs.begin(); it != args.kvpairs.end(); it++) {
		vector<string> path = Utils::split(it->first, '.');

		switch (path.size()) {
		case 1:
			j[path[0]] = it->second;
			break;
		case 2:
			j[path[0]][path[1]] = it->second;
			break;
		case 3:
			j[path[0]][path[1]][path[2]] = it->second;
			break;
		case 4:
			j[path[0]][path[1]][path[2]][path[3]] = it->second;
			break;
		case 5:
			j[path[0]][path[1]][path[2]][path[3]][path[4]] = it->second;
			break;
		case 6:
			j[path[0]][path[1]][path[2]][path[3]][path[4]][path[5]] = it->second;
			break;
		case 7:
			j[path[0]][path[1]][path[2]][path[3]][path[4]][path[5]][path[6]] = it->second;
			break;
		case 8:
			j[path[0]][path[1]][path[2]][path[3]][path[4]][path[5]][path[6]][path[7]] = it->second;
			break;
		case 9:
			j[path[0]][path[1]][path[2]][path[3]][path[4]][path[5]][path[6]][path[7]][path[8]] = it->second;
			break;
		case 10:
			j[path[0]][path[1]][path[2]][path[3]][path[4]][path[5]][path[6]][path[7]][path[8]][path[9]] = it->second;
			break;
		default:
			assert(0);
			break;
		}
	}

	// write it out
	args.writeJson(j);
	cout << "Done!" << endl << endl;
}



