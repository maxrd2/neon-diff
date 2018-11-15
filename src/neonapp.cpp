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

#include <algorithm>
#include <cassert>

using namespace std;

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
const char *colorDiffIntro = colorCyan;
const char *colorFileIntro = colorWhite;
const char *colorBlockInfo = colorBlue;
const char *colorLineDel = colorRed;
const char *colorLineAdd = colorGreen;
const char *colorLineContext = colorReset;

// inverts background and foreground colors
const char *highlightOn = "\33[7m";
const char *highlightOff = "\33[27m";
const char *highlightReset = highlightOff;


NeonApp * NeonApp::instance_ = nullptr;

class Block {
public:
	Block()
		: aBuf(nullptr),
		  aEnd(nullptr),
		  bBuf(nullptr),
		  bEnd(nullptr),
		  len(0)
	{}

	const char *aBuf;
	const char *aEnd;
	const char *bBuf;
	const char *bEnd;
	int len;
};

typedef std::list<Block *> BlockList;



NeonApp::NeonApp(FILE *inputStream, FILE *outputStream)
	: input_(inputStream),
	  output_(outputStream),
	  buf_(nullptr),
	  bufLen_(0),
	  bufSize_(8192),
	  line_(nullptr),
	  lineLen_(0),
	  blockRem_(nullptr),
	  blockAdd_(nullptr),
	  outLineStart_(true),
	  outLineIndent_(true),
	  selectedColor_(colorReset),
	  selectedHighlight_(highlightReset),
	  printedColor_(nullptr),
	  printedHighlight_(nullptr)
{
	buf_ = static_cast<char *>(malloc(bufSize_));
}

NeonApp::~NeonApp()
{
	if(buf_) {
		free(buf_);
		bufSize_ = bufLen_ = 0;
	}
	line_ = nullptr;
	lineLen_ = 0;
	blockRem_ = nullptr;
	blockAdd_ = nullptr;
}

// main loop
void
NeonApp::processInput()
{
	setColor(colorDiffIntro);

	bool inputHasAnsi = false;

	bool inBlock = false;
	while(readLine()) {
		const char *ch = line_;
		if(*ch == '\33') {
			// skip existing ansi colors
			inputHasAnsi = true;
			while(*ch++ != 'm');
		}

		if((blockRem_ || blockAdd_) && *ch != '-' && *ch != '+') {
			outputDiff();
			setColor(colorLineContext);
		} else if(*ch == '@') {
			stripAnsi();
			inBlock = true;
			setColor(colorBlockInfo);
		} else if(inBlock) {
			stripAnsi();
			if(*line_ == '-') {
				if(!blockRem_)
					blockRem_ = line_;
				continue;
			} else if(*line_ == '+') {
				if(!blockAdd_)
					blockAdd_ = line_;
				continue;
			} else if(*line_ == ' ') {
				setColor(colorLineContext);
			} else {
				inBlock = false;
			}
		}

		bool preserveAnsi = false;

		if(!inBlock) {
			if((*ch == '+' || *ch == '-') && *ch == ch[1] && *ch == ch[2]) {
				stripAnsi();
				setColor(colorFileIntro);
			} else {
				if(inputHasAnsi)
					preserveAnsi = true; // we want to keep exisitng colors from input here
				else
					setColor(colorDiffIntro);
			}
		}

		if(preserveAnsi) {
			while(lineLen_--)
				fputc(*line_++, output_);
		} else {
			while(lineLen_--)
				printChar(*line_++);
			setColor(colorLineContext);
		}

		bufLen_ = 0;
	}

	if(blockRem_ || blockAdd_) {
		outputDiff();
		setColor(colorLineContext);
	}

	setColor(colorReset);
}

// reading and processing input
bool
NeonApp::readLine()
{
	line_ = &buf_[bufLen_];
	lineLen_ = 0;

	while(!feof(input_)) {
		if(bufLen_ == bufSize_) {
			char *bufOld = buf_;
			bufSize_ *= 2;
			buf_ = static_cast<char *>(realloc(buf_, bufSize_));
			// change pointers to new buf address
			line_ = buf_ + (line_ - bufOld);
			if(blockRem_)
				blockRem_ = buf_ + (blockRem_ - bufOld);
			if(blockAdd_)
				blockAdd_ = buf_ + (blockAdd_ - bufOld);
		}

		int ch = fgetc(input_);
		buf_[bufLen_] = ch;

		bufLen_++;
		lineLen_++;

		if(ch == '\n')
			break;
	}

	return !feof(input_);
}

void
NeonApp::stripAnsi()
{
	char *left = line_;
	char *right = line_;

	for(int i = 0; i < lineLen_; i++) {
		while(*right == '\33') {
			while(lineLen_-- && bufLen_-- && *right++ != 'm');
		}
		*left++ = *right++;
	}
}

void
NeonApp::outputBlock(const char *block, const char *blockEnd)
{
	while(block < blockEnd)
		printChar(*block++);
}

Block *
NeonApp::longestMatch(const char *rem, const char *remEnd, const char *add, const char *addEnd)
{
	Block *best = new Block();
	Block cur;

	const char *bSave = add;

	while(remEnd - rem > best->len) {
		while(addEnd - add > best->len) {
			cur.aBuf = cur.aEnd = rem;
			cur.bBuf = cur.bEnd = add;
			cur.len = 0;

			int i = 0;
			int j = 0;
			for(;;) {
				// skip spaces
				while(&rem[i] < remEnd && (rem[i] == ' ' || rem[i] == '\t' || rem[i] == '\n' || rem[i - 1] == '\n'))
					i++;
				while(&add[j] < addEnd && (add[j] == ' ' || add[j] == '\t' || add[j] == '\n' || add[j - 1] == '\n'))
					j++;

				if(&rem[i] >= remEnd || &add[j] >= addEnd || rem[i] != add[j])
					break;

				cur.len++;

				i++;
				j++;

				// skip spaces
				while(&rem[i] < remEnd && (rem[i] == ' ' || rem[i] == '\t' || rem[i] == '\n' || rem[i - 1] == '\n'))
					i++;
				while(&add[j] < addEnd && (add[j] == ' ' || add[j] == '\t' || add[j] == '\n' || add[j - 1] == '\n'))
					j++;

				assert(&rem[i] <= remEnd);
				assert(&add[j] <= addEnd);
				cur.aEnd = &rem[i];
				cur.bEnd = &add[j];
			}

			if(cur.len > best->len) {
				best->aBuf = cur.aBuf;
				best->aEnd = cur.aEnd;
				best->bBuf = cur.bBuf;
				best->bEnd = cur.bEnd;
				best->len = cur.len;
			}

			add++;
		}
		rem++;
		add = bSave;
	}

	if(best->len)
		return best;

	delete best;
	return nullptr;
}

BlockList
NeonApp::compareBlocks(const char *remStart, const char *remEnd, const char *addStart, const char *addEnd)
{
	BlockList list;
	Block *longest = longestMatch(remStart, remEnd, addStart, addEnd);
	if(longest) {
		list = compareBlocks(remStart, longest->aBuf, addStart, longest->bBuf);
		list.push_back(longest);

		BlockList tail = compareBlocks(longest->aEnd, remEnd, longest->bEnd, addEnd);
		list.insert(list.end(), tail.begin(), tail.end());
	}

	return list;
}

void
NeonApp::outputDiff()
{
	if(!blockAdd_) {
		setColor(colorLineDel);
		outputBlock(blockRem_, line_);
		blockRem_ = nullptr;
		return;
	}
	if(!blockRem_) {
		setColor(colorLineAdd);
		outputBlock(blockAdd_, line_);
		blockAdd_ = nullptr;
		return;
	}

	BlockList blocks = compareBlocks(blockRem_ + 1, blockAdd_, blockAdd_ + 1, line_);

	setColor(colorLineDel);
	const char *start = blockRem_;
	for(BlockList::iterator i = blocks.begin(); i != blocks.end(); ++i) {
		setHighlight(highlightOn);
		outputBlock(start, (*i)->aBuf);
		setHighlight(highlightOff);
		outputBlock((*i)->aBuf, (*i)->aEnd);
		start = (*i)->aEnd;
	}
	setHighlight(highlightOn);
	outputBlock(start, blockAdd_);

	setColor(colorLineAdd);
	start = blockAdd_;
	for(BlockList::iterator i = blocks.begin(); i != blocks.end(); ++i) {
		setHighlight(highlightOn);
		outputBlock(start, (*i)->bBuf);
		setHighlight(highlightOff);
		outputBlock((*i)->bBuf, (*i)->bEnd);
		start = (*i)->bEnd;
	}
	setHighlight(highlightOn);
	outputBlock(start, line_);

	setHighlight(highlightOff);
	blockAdd_ = blockRem_ = nullptr;
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
NeonApp::printChar(const char ch)
{
	if(ch == '\n') {
		fputc('\n', output_);

		// when using some pagers ANSI codes get reset on newline, so we're going to reset them
		if(printedHighlight_ != highlightReset)
			printedHighlight_ = nullptr;
		if(printedColor_ != colorReset)
			printedColor_ = nullptr;

		outLineStart_ = outLineIndent_ = 1;
		return;
	} else {
		outLineIndent_ = outLineStart_ || (outLineIndent_ && (ch == ' ' || ch == '\t'));

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

		fputc(ch, output_);

		outLineStart_ = 0;
	}
}
