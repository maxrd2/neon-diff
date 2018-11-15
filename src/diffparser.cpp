#include "diffparser.h"

#include "neonapp.h"
#include "colors.h"

#include <stdlib.h>
#include <algorithm>
#include <cassert>


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



DiffParser::DiffParser(FILE *inputStream)
	: input_(inputStream),
	  buf_(nullptr),
	  bufLen_(0),
	  bufSize_(8192),
	  line_(nullptr),
	  lineLen_(0),
	  blockRem_(nullptr),
	  blockAdd_(nullptr)
{
	buf_ = static_cast<char *>(malloc(bufSize_));
}

DiffParser::~DiffParser()
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

void
DiffParser::processInput()
{
	app->setColor(colorDiffIntro);

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
			app->setColor(colorLineContext);
		} else if(*ch == '@') {
			stripAnsi();
			inBlock = true;
			app->setColor(colorBlockInfo);
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
				app->setColor(colorLineContext);
			} else {
				inBlock = false;
			}
		}

		bool preserveAnsi = false;

		if(!inBlock) {
			if((*ch == '+' || *ch == '-') && *ch == ch[1] && *ch == ch[2]) {
				stripAnsi();
				app->setColor(colorFileIntro);
			} else {
				if(inputHasAnsi)
					preserveAnsi = true; // we want to keep exisitng colors from input here
				else
					app->setColor(colorDiffIntro);
			}
		}

		if(preserveAnsi) {
			while(lineLen_--)
				app->printCharNoAnsi(*line_++);
		} else {
			while(lineLen_--)
				app->printChar(*line_++);
			app->setColor(colorLineContext);
		}

		bufLen_ = 0;
	}

	if(blockRem_ || blockAdd_) {
		outputDiff();
		app->setColor(colorLineContext);
	}

	app->setColor(colorReset);
}

bool
DiffParser::readLine()
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
DiffParser::stripAnsi()
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

Block *
DiffParser::longestMatch(const char *rem, const char *remEnd, const char *add, const char *addEnd)
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
DiffParser::compareBlocks(const char *remStart, const char *remEnd, const char *addStart, const char *addEnd)
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
DiffParser::outputDiff()
{
	if(!blockAdd_) {
		app->setColor(colorLineDel);
		app->outputBlock(blockRem_, line_);
		blockRem_ = nullptr;
		return;
	}
	if(!blockRem_) {
		app->setColor(colorLineAdd);
		app->outputBlock(blockAdd_, line_);
		blockAdd_ = nullptr;
		return;
	}

	BlockList blocks = compareBlocks(blockRem_ + 1, blockAdd_, blockAdd_ + 1, line_);

	app->setColor(colorLineDel);
	const char *start = blockRem_;
	for(BlockList::iterator i = blocks.begin(); i != blocks.end(); ++i) {
		app->setHighlight(highlightOn);
		app->outputBlock(start, (*i)->aBuf);
		app->setHighlight(highlightOff);
		app->outputBlock((*i)->aBuf, (*i)->aEnd);
		start = (*i)->aEnd;
	}
	app->setHighlight(highlightOn);
	app->outputBlock(start, blockAdd_);

	app->setColor(colorLineAdd);
	start = blockAdd_;
	for(BlockList::iterator i = blocks.begin(); i != blocks.end(); ++i) {
		app->setHighlight(highlightOn);
		app->outputBlock(start, (*i)->bBuf);
		app->setHighlight(highlightOff);
		app->outputBlock((*i)->bBuf, (*i)->bEnd);
		start = (*i)->bEnd;
	}
	app->setHighlight(highlightOn);
	app->outputBlock(start, line_);

	app->setHighlight(highlightOff);
	blockAdd_ = blockRem_ = nullptr;
}
