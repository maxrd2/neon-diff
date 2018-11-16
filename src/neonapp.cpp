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

bool NeonApp::ignoreSpaces_ = false;
int NeonApp::indentWidth_ = 0;
const char *NeonApp::tabCharacter_ = " ";
int NeonApp::tabWidth_ = 4;


NeonApp::NeonApp(FILE *inputStream, FILE *outputStream)
	: parser_(new DiffParser(inputStream)),
	  output_(outputStream),
	  outputOnStart_(true),
	  outputOnIndent_(true),
	  outputIndex_(0),
	  outputSpaces_(0),
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

	outputOnStart_ = outputOnIndent_ = true;
	outputIndex_ = 0;
	outputSpaces_ = 0;
}

void
NeonApp::printAnsiCodes()
{
	if(printedColor_ != selectedColor_) {
		printedColor_ = selectedColor_;
		fputs(selectedColor_, output_);
	}
	if(ignoreSpaces_ ? outputOnIndent_ : outputOnStart_) {
		// we don't want to highlight first character/spaces
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
		outputOnIndent_ = outputOnStart_ || (outputOnIndent_ && (ch == ' ' || ch == '\t'));
		if(writeAnsi)
			printAnsiCodes();
		if(ch == '\t') {
			const int charsToTabStop = tabWidth_ - (outputIndex_ - 1) % tabWidth_;
			fputs(app->tabCharacter_, output_);
			for(int i = 1; i < charsToTabStop; i++)
				fputc(' ', output_);
			outputIndex_ += charsToTabStop;
			outputSpaces_ = 0;
		} else {
			if(indentWidth_ && outputOnIndent_) {
				if(ch == ' ' && !outputOnStart_) {
					outputSpaces_++;
					if(outputSpaces_ == indentWidth_) {
						const int move = tabWidth_ - indentWidth_;
						if(move > 0)
							fprintf(output_, "\33[%dC", move);
						else
							fprintf(output_, "\33[%dD", -move);
						outputSpaces_ = 0;
					}
				} else {
					outputSpaces_ = 0;
				}
			}
			fputc(ch, output_);
			outputIndex_++;
		}
		outputOnStart_ = false;
	}
}
