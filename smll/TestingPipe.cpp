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
#include "TestingPipe.hpp"
#include "Config.hpp"

namespace smll {

	static TestingPipe g_testingPipe;

	TestingPipe::TestingPipe() 
		: hPipe(INVALID_HANDLE_VALUE)
	{
	}

	TestingPipe::~TestingPipe()
	{
		ClosePipe();
	}

	void TestingPipe::OpenPipe()
	{
		if (hPipe == INVALID_HANDLE_VALUE)
		{
			hPipe = CreateFile(TEXT("\\\\.\\pipe\\SlobsTestPipe"),
				GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
				NULL);
		} 
	}

	void TestingPipe::ClosePipe()
	{
		if (hPipe != INVALID_HANDLE_VALUE)
		{
			SendString("Shutting down test server");
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;
		}
	}

	TestingPipe& TestingPipe::singleton()
	{
		return g_testingPipe;
	}

	int TestingPipe::SendString(const std::string s)
	{
		OpenPipe();

		DWORD dwWritten = 0;
		if (hPipe != INVALID_HANDLE_VALUE)
		{
			WriteFile(hPipe, s.c_str(),	(DWORD)(s.length() + 1), 
				&dwWritten,	NULL);
			FlushFileBuffers(hPipe);
		}

		return (int)dwWritten;
	}

}