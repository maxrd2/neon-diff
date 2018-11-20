#ifndef DIFFPARSE_H
#define DIFFPARSE_H

#include <stdio.h>
#include <list>

#define LINE_HANDLER_SIZE 6

class Match {
public:
	Match(const char *rem, const char *remEnd, const char *add, const char *addEnd, int len);
	Match();
	const char *rem_;
	const char *remEnd_;
	const char *add_;
	const char *addEnd_;
	int len_;
};

typedef std::list<Match> MatchList;

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

	bool handlerForLine(const char *line, const char *id, int n);

	void handleFileInfoLine();
	void handleRangeInfoLine();

	void handleContextLine();
	void handleRemLine();
	void handleAddLine();

	void handleGenericLine();

	void printBlock(const char id, const char *block, const char *blockEnd);
	void processBlock();

	void stripLineAnsi(int stripIndent = 0);

	Match longestMatch(const char *rem, const char *remEnd, const char *add, const char *addEnd);
	MatchList compareBlocks(const char *a, const char *aEnd, const char *b, const char *bEnd);

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

	typedef void (DiffParser::* LineHandlerCallback)();

	static const struct LineHandler {
		const char *identifier;
		LineHandlerCallback callback;
		bool blockLine;
	} lineHandler_[LINE_HANDLER_SIZE];
};

#endif // DIFFPARSE_H
