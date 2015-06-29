/*
 *  AdExpSim -- Simulator for the AdExp model
 *  Copyright (C) 2015  Andreas Stöckel
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.d.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdint>
#include <sstream>
#include <vector>
#include <limits>

#include "Terminal.hpp"

std::string Terminal::color(int color, bool bright) const
{
	if (!useColor) {
		return std::string{};
	}
	std::stringstream ss;
	ss << "\x1b[";
	if (bright) {
		ss << "1;";
	}
	ss << color << "m";
	return ss.str();
}

std::string Terminal::background(int color) const
{
	if (!useColor) {
		return std::string{};
	}
	std::stringstream ss;
	ss << "\x1b[";
	ss << (color + 10) << "m";
	return ss.str();
}

std::string Terminal::rgb(uint8_t r, uint8_t g, uint8_t b,
                          bool background) const
{
	if (!useColor) {
		return std::string{};
	}

	std::stringstream ss;
#if 0
	ss << "\x1b[" << (background ? "48" : "38") << ";2;" << (int)r << ";"
	   << (int)g << ";" << (int)b << "m";
#else
	size_t code = 0;
	if (r == g && g == b) {
		// Issue a BW color
		if (r == 0) {
			code = 16;
		} else {
			code = 232 + r * 24 / 256;
		}
	} else {
		// Issue a color from the 6 x 6 x 6 color cube
		uint8_t offsB = b * 6 / 256;
		uint8_t offsG = g * 6 / 256;
		uint8_t offsR = r * 6 / 256;
		code = 16 + offsR * 36 + offsG * 6 + offsB;
	}

	// Generate the output code
	ss << "\x1b[" << (background ? "48" : "38") << ";5;" << code << "m";
#endif
	return ss.str();
}

std::string Terminal::bright() const
{
	if (!useColor) {
		return std::string{};
	}
	return "\x1b[1m";
}

std::string Terminal::italic() const
{
	if (!useColor) {
		return std::string{};
	}
	return "\x1b[3m";
}

std::string Terminal::underline() const
{
	if (!useColor) {
		return std::string{};
	}
	return "\x1b[4m";
}

std::string Terminal::reset() const
{
	if (!useColor) {
		return std::string{};
	}
	return "\x1b[0m";
}

