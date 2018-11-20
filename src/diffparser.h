#ifndef DIFFPARSE_H
#define DIFFPARSE_H

#include <stdio.h>
#include <list>
#include <vector>

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

class HalfMatch {
public:
	HalfMatch(const char *rem, const char *remEnd, int len);
	HalfMatch(const Match &match);
	HalfMatch();
	inline bool operator<(const HalfMatch &other) const { return len_ < other.len_; }
	const char *rem_;
	const char *remEnd_;
	int len_;
};

typedef std::list<HalfMatch> HalfMatchList;

class DiffParser
{
public:
	DiffParser(FILE *inputStream);
	virtual ~DiffParser();

	void processInput();

	bool readLine();

protected:
	void resizeBuffers();
	void resetBuffers();

	bool handlerForLine(const char *line, const char *id, int n);

	void handleFileInfoLine();
	void handleRangeInfoLine();

	void handleContextLine();
	void handleRemLine();
	void handleAddLine();

	void handleGenericLine();

	void printBlock(const char id, const char *block, const char *blockEnd);
	void processBlock();

	void stripLineAnsi(int stripIndent = 0, bool writeToAlt = false, bool moveToAlt = false);

	void buildMatchCache(const char *rem, const char *remEnd, const char *add, const char *addEnd);
	Match longestMake(const char *rem, const char *remEnd, const char *add, const char *addEnd, const int len);
	void cacheClip(const char *rem, const char *remEnd);
	Match longestMatch(const char *rem, const char *remEnd, const char *add, const char *addEnd);
	MatchList compareBlocks(const char *a, const char *aEnd, const char *b, const char *bEnd);

	void printLineNoAnsi(int length = -1);

private:
	FILE *input_;

	char *buf_;
	int bufLen_;
	int bufSize_;
	char *alt_;
	int altLen_;
	int altSize_;

	char *line_;
	int lineLen_;

	bool inBlock_;
	const char *blockRem_;
	const char *blockRemEnd_;
	const char *blockAdd_;
	const char *blockAddEnd_;

	HalfMatchList cache_;

	typedef void (DiffParser::* LineHandlerCallback)();

	static const struct LineHandler {
		const char *identifier;
		LineHandlerCallback callback;
		bool blockLine;
	} lineHandler_[LINE_HANDLER_SIZE];
};

#endif // DIFFPARSE_H
