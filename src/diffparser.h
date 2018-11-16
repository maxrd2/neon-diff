#ifndef DIFFPARSE_H
#define DIFFPARSE_H

#include <stdio.h>
#include <list>

#define LINE_HANDLER_SIZE 6

class Block;
typedef std::list<Block *> BlockList;

class DiffParser
{
public:
	DiffParser(FILE *inputStream);
	virtual ~DiffParser();

	void processInput();

	bool readLine();

protected:
	void resizeBuffer();
	void resetBuffer();

	static bool handlerForLine(const char *line, const char *id, int n);

	static void handleFileInfoLine(DiffParser *parser);
	static void handleRangeInfoLine(DiffParser *parser);

	static void handleContextLine(DiffParser *parser);
	static void handleRemLine(DiffParser *parser);
	static void handleAddLine(DiffParser *parser);

	static void handleGenericLine(DiffParser *parser);

	void printBlock(const char id, const char *block, const char *blockEnd);
	void processBlock();

	void stripLineAnsi(int stripIndent = 0);

	Block * longestMatch(const char *rem, const char *remEnd, const char *add, const char *addEnd);
	BlockList compareBlocks(const char *a, const char *aEnd, const char *b, const char *bEnd);

	void printLineNoAnsi(int length = -1);

private:
	FILE *input_;

	char *buf_;
	int bufLen_;
	int bufSize_;

	char *line_;
	int lineLen_;

	bool inBlock_;
	const char *blockRem_;
	const char *blockAdd_;

	typedef void (*LineHandlerCallback)(DiffParser *parser);

	static struct LineHandler {
		const char *identifier;
		LineHandlerCallback callback;
		bool blockLine;
	} lineHandler_[LINE_HANDLER_SIZE];
};

#endif // DIFFPARSE_H
