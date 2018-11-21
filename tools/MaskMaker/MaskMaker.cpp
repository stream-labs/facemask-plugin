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
#include "command_create.h"
#include "command_addres.h"
#include "command_addpart.h"
#include "command_import.h"
#include "command_inspect.h"
#include "command_morph_import.h"
#include "command_merge.h"
#include "command_tweak.h"
#include "command_depends.h"


using namespace std;



int main(int argc, char** argv) {

	// parse arguments
	Args args(argc, argv);
	if (args.failed)
		return -1;

	// run command
	if (args.command == "create")
		command_create(args);
	else if (args.command == "addres")
		command_addres(args);
	else if (args.command == "addpart")
		command_addpart(args);
	else if (args.command == "merge")
		command_merge(args);
	else if (args.command == "import")
		command_import(args);
	else if (args.command == "inspect")
		inspect::command_inspect(args);
	else if (args.command == "morphimport" || args.command == "mi")
		command_morph_import(args);
	else if (args.command == "tweak")
		command_tweak(args);
	else if (args.command == "depends")
		command_depends(args);

	
	//getchar();
    return 0;
}

