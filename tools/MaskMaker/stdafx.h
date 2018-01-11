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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <sstream>

#include "json.hpp"
#include "base64.h"
#include "utils.h"
#include "args.h"

// Assimp : open asset importer
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/quaternion.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// AVIR : Advanced image resizer
#include "avir.h"

#include <zlib.h>


// Float macros
#define FLT_EQ(X,Y)		(((int)((X) * 10000.0f + 0.5f))==((int)((Y) * 10000.0f + 0.5f)))
#define FLT_NEQ(X,Y)	(!FLT_EQ(X,Y))
#define IS_POWER_2(X)	(((X) > 0) && (((X) & ((X)-1)) == 0))


