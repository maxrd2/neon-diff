/*
	neon-diff - Application to colorify, highlight and beautify unified diffs.

	Copyright (C) 2018 - Mladen Milinkovic <maxrd2@smoothware.net>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "colors.h"

// ansi colors
const char *colorRed = "\33[91m";
const char *colorGreen = "\33[92m";
const char *colorYellow = "\33[93m";
const char *colorBlue = "\33[94m";
const char *colorMagenta = "\33[95m";
const char *colorCyan = "\33[96m";
const char *colorWhite = "\33[97m";
const char *colorReset = "\33[m";

// colors used for diff parts
const char *colorFileInfo = colorWhite;
const char *colorBlockRange = colorCyan;
const char *colorBlockHeading = colorBlue;
const char *colorLineDel = colorRed;
const char *colorLineAdd = colorGreen;
const char *colorLineContext = colorReset;

// inverts background and foreground colors
const char *highlightOn = "\33[7m";
const char *highlightOff = "\33[27m";
const char *highlightReset = highlightOff;
