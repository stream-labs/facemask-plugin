/*
* Face Masks for SlOBS
* smll - streamlabs machine learning library
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
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <iostream>

int main(int argc, char ** argv) {
	
	char ** argvCopy = new char*[sizeof(char *) * (argc + 2)];
	size_t i;
	for (i = 0; i < argc; i++) {
		argvCopy[i] = argv[i];
	}

	argvCopy[i] = "-c"; // enable colored output
	argvCopy[i + 1] = "-v"; // enable detailed results
	//run
    return RUN_ALL_TESTS(argc + 2, argvCopy);

}