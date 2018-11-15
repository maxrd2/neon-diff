#include "diffparser.h"

#include "neonapp.h"
#include "colors.h"

#include <stdlib.h>
#include <algorithm>
#include <cassert>


#define BUFFER_SIZE_INIT 8192
// when buffer becomes too small its size is doubled, unless bigger than this
#define BUFFER_MAX_SIZE_INC 8192 * 1024


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

DiffParser::LineHandler DiffParser::lineHandler_[LINE_HANDLER_SIZE] = {
	// diff header lines
	{"---", DiffParser::handleFileInfoLine, false},
	{"+++", DiffParser::handleFileInfoLine, false},
	{"@@", DiffParser::handleRangeInfoLine, false},
	// block/range diff lines
	{" ", DiffParser::handleContextLine, false},
	{"-", DiffParser::handleRemLine, true},
	{"+", DiffParser::handleAddLine, true}
};


DiffParser::DiffParser(FILE *inputStream)
	: input_(inputStream),
	  buf_(nullptr),
	  bufLen_(0),
	  bufSize_(BUFFER_SIZE_INIT),
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

/*static*/ bool
DiffParser::handlerForLine(const char *line, const char *id, int n)
{
	while(n && *line && *id) {
		if(*line == '\33') { // skip ansi chars
			while(n-- && *line++ != 'm');
			continue;
		}
		n--;
		if(*line++ != *id++)
			return false;
	}

	return *id == 0;
}

void
DiffParser::processInput()
{
	inBlock_ = false;

	while(readLine()) {
		// find a line handler
		int i = 0;
		while(i < LINE_HANDLER_SIZE && !handlerForLine(line_, lineHandler_[i].identifier, lineLen_))
			i++;

		if(i == LINE_HANDLER_SIZE) {
			// no handler found, use generic one
			handleGenericLine(this);
		} else {
			if(inBlock_ && !lineHandler_[i].blockLine)
				processBlock();

			inBlock_ = lineHandler_[i].blockLine;

			lineHandler_[i].callback(this);

			if(!inBlock_)
				resetBuffer();
		}
	}

	if(inBlock_)
		processBlock();
}

void
DiffParser::resetBuffer()
{
	bufLen_ = 0;
	line_ = nullptr;
	lineLen_ = 0;
	blockRem_ = nullptr;
	blockAdd_ = nullptr;
}

void
DiffParser::resizeBuffer()
{
	char *buf = buf_;
	bufSize_ += bufSize_ > BUFFER_MAX_SIZE_INC ? BUFFER_MAX_SIZE_INC : bufSize_;
	buf_ = static_cast<char *>(realloc(buf_, bufSize_));

	// change pointers to new buf address
	line_ = buf_ + (line_ - buf);
	if(blockRem_)
		blockRem_ = buf_ + (blockRem_ - buf);
	if(blockAdd_)
		blockAdd_ = buf_ + (blockAdd_ - buf);
}

bool
DiffParser::readLine()
{
	line_ = &buf_[bufLen_];
	lineLen_ = 0;

	while(!feof(input_)) {
		if(bufLen_ >= bufSize_)
			resizeBuffer();

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
DiffParser::stripLineAnsi()
{
	char *left = line_;
	char *right = line_;

	for(int i = 0; i < lineLen_;) {
		if(*right == '\33') { // skip ansi chars
			do {
				lineLen_--;
				bufLen_--;
			} while(i < lineLen_ && *right++ != 'm');
			continue;
		}
		i++;
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
DiffParser::processBlock()
{
	inBlock_ = false;

	if(!blockAdd_) {
		app->setColor(colorLineDel);
		app->printBlock(blockRem_, line_);
		blockRem_ = nullptr;
		return;
	}
	if(!blockRem_) {
		app->setColor(colorLineAdd);
		app->printBlock(blockAdd_, line_);
		blockAdd_ = nullptr;
		return;
	}

	BlockList blocks = compareBlocks(blockRem_ + 1, blockAdd_, blockAdd_ + 1, line_);

	app->setColor(colorLineDel);
	const char *start = blockRem_;
	for(BlockList::iterator i = blocks.begin(); i != blocks.end(); ++i) {
		app->setHighlight(highlightOn);
		app->printBlock(start, (*i)->aBuf);
		app->setHighlight(highlightOff);
		app->printBlock((*i)->aBuf, (*i)->aEnd);
		start = (*i)->aEnd;
	}
	app->setHighlight(highlightOn);
	app->printBlock(start, blockAdd_);

	app->setColor(colorLineAdd);
	start = blockAdd_;
	for(BlockList::iterator i = blocks.begin(); i != blocks.end(); ++i) {
		app->setHighlight(highlightOn);
		app->printBlock(start, (*i)->bBuf);
		app->setHighlight(highlightOff);
		app->printBlock((*i)->bBuf, (*i)->bEnd);
		start = (*i)->bEnd;
	}
	app->setHighlight(highlightOn);
	app->printBlock(start, line_);

	app->setHighlight(highlightOff);
	blockAdd_ = blockRem_ = nullptr;
}

/*!
 * \brief Print \p (part of) current line and update internal line pointer and length. Ansi sequences will not be printed.
 * \param length how many bytes to print (-1 for whole line)
 */
void
DiffParser::printLineNoAnsi(int length/* = -1 */)
{
	if(length == -1 || length > lineLen_)
		length = lineLen_;

	if(!length)
		return;

	lineLen_ -= length;

	const char *lineEnd = line_ + length;

	while(line_ < lineEnd) {
		if(*line_ == '\33') { // skip ansi chars
			while(line_ < lineEnd && *line_++ != 'm');
			continue;
		}
		app->printChar(*line_++);
	}
}

/*static*/ void
DiffParser::handleFileInfoLine(DiffParser *parser)
{
	// print '---' and '+++' lines

	app->setHighlight(highlightOff);
	app->setColor(colorFileInfo);

	parser->printLineNoAnsi();

	app->setColor(colorReset);
	app->setHighlight(highlightReset);
}

/*static*/ void
DiffParser::handleRangeInfoLine(DiffParser *parser)
{
	// print '@@' lines

	app->setHighlight(highlightOff);

	app->setColor(colorBlockRange);
	int at = 0;
	int rangeLen = 0;
	while(at < 4 && rangeLen < parser->lineLen_) {
		if(parser->line_[rangeLen++] == '@')
			at++;
	}
	parser->printLineNoAnsi(rangeLen);

	app->setColor(colorBlockHeading);
	parser->printLineNoAnsi();

	app->setColor(colorReset);
	app->setHighlight(highlightReset);
}

/*static*/ void
DiffParser::handleContextLine(DiffParser *parser)
{
	// print ' ' lines inside diff block

	app->setHighlight(highlightOff);
	app->setColor(colorLineContext);

	parser->printLineNoAnsi();

	app->setColor(colorReset);
	app->setHighlight(highlightReset);
}

/*static*/ void
DiffParser::handleRemLine(DiffParser *parser)
{
	// handle '-' lines inside diff block

	if(parser->blockAdd_) // when '-' block comes after '+' block, we have to process
		parser->processBlock();

	if(!parser->blockRem_)
		parser->blockRem_ = parser->line_;

	parser->stripLineAnsi();

	// we are just preparing block buffers, they will be printed in processBlock()
}

/*static*/ void
DiffParser::handleAddLine(DiffParser *parser)
{
	// handle '+' lines inside diff block

	if(!parser->blockAdd_)
		parser->blockAdd_ = parser->line_;

	parser->stripLineAnsi();

	// we are just preparing block buffers, they will be printed in processBlock()
}

/*static*/ void
DiffParser::handleGenericLine(DiffParser *parser)
{
	app->setColor(colorReset);
	app->setHighlight(highlightReset);
	app->printAnsiCodes();

	while(parser->lineLen_--)
		app->printCharNoAnsi(*parser->line_++);
}
