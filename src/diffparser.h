#ifndef DIFFPARSE_H
#define DIFFPARSE_H

#include <stdio.h>
#include <list>

class Block;
typedef std::list<Block *> BlockList;

class DiffParser
{
public:
	DiffParser(FILE *inputStream);
	virtual ~DiffParser();

	void processInput();

	bool readLine();
	void stripAnsi();

	Block * longestMatch(const char *rem, const char *remEnd, const char *add, const char *addEnd);
	BlockList compareBlocks(const char *a, const char *aEnd, const char *b, const char *bEnd);
	void outputDiff();

private:
	FILE *input_;

	char *buf_;
	int bufLen_;
	int bufSize_;

	char *line_;
	int lineLen_;

	const char *blockRem_;
	const char *blockAdd_;
};

#endif // DIFFPARSE_H
