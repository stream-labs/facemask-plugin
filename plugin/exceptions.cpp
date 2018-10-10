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

#include "exceptions.h"

Plugin::io_error::io_error(std::string p_message, std::string p_file)
	: runtime_error(p_message), file(p_file) {
	
}

Plugin::io_error::io_error(std::string p_file)
	: io_error(p_file, p_file) {

}

Plugin::file_not_found_error::file_not_found_error(std::string p_message,
	std::string p_file)
	: io_error(p_message, p_file) {

}

Plugin::file_not_found_error::file_not_found_error(std::string p_file)
	: file_not_found_error(p_file, p_file) {

}
