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

#include "neonapp.h"
#include "diffparser.h"
#include "colors.h"

using namespace std;



NeonApp::NeonApp(FILE *inputStream, FILE *outputStream)
	: parser_(new DiffParser(inputStream)),
	  output_(outputStream),
	  outLineStart_(true),
	  outLineIndent_(true),
	  selectedColor_(colorReset),
	  selectedHighlight_(highlightReset),
	  printedColor_(nullptr),
	  printedHighlight_(nullptr)
{
}

NeonApp::~NeonApp()
{
	delete parser_;
}

// output display and handling
void
NeonApp::setColor(const char *color)
{
	selectedColor_ = color;
}

void
NeonApp::setHighlight(const char *highlight)
{
	selectedHighlight_ = highlight;
}

void
NeonApp::printNewLine()
{
	fputc('\n', output_);

	// when using some pagers ANSI codes get reset on newline, so we're going to reset them
	if(printedHighlight_ != highlightReset)
		printedHighlight_ = nullptr;
	if(printedColor_ != colorReset)
		printedColor_ = nullptr;

	outLineStart_ = outLineIndent_ = true;
}

void
NeonApp::printAnsiCodes()
{
	if(printedColor_ != selectedColor_) {
		printedColor_ = selectedColor_;
		fputs(selectedColor_, output_);
	}
	if(outLineStart_) {
		// we don't want to highlight first character
		if(printedHighlight_ != highlightOff) {
			printedHighlight_ = highlightOff;
			fputs(highlightOff, output_);
		}
	} else if(printedHighlight_ != selectedHighlight_) {
		printedHighlight_ = selectedHighlight_;
		fputs(selectedHighlight_, output_);
	}
}

void
NeonApp::printChar(const char ch, bool writeAnsi/* = true*/)
{
	if(ch == '\n') {
		printNewLine();
	} else {
		outLineIndent_ = outLineStart_ || (outLineIndent_ && (ch == ' ' || ch == '\t'));
		if(writeAnsi)
			printAnsiCodes();
		fputc(ch, output_);
		outLineStart_ = false;
	}
}
